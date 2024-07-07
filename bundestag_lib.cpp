#include "bundestag_lib.h"

std::map<int, std::string> stateMap =
{
    {SH, "Schleswig-Holstein"},
    {MV, "Mecklenburg-Vorpommern"},
    {HH, "Hamburg"},
    {NI, "Niedersachsen"},
    {HB, "Bremen"},
    {BB, "Brandenburg"},
    {ST, "Sachsen-Anhalt"},
    {BE, "Berlin"},
    {NW, "Nordrhein-Westfalen"},
    {SN, "Sachsen"},
    {HE, "Hessen"},
    {TH, "Thueringen"},
    {RP, "Rheinland-Pfalz"},
    {BY, "Bayern"},
    {BW, "Baden-Wuerttemberg"},
    {SL, "Saarland"}
};
vector<std::string> StateData::party_names = {};


void SainteLague::init(int* distribution, int parties, int seats)
{
    this->init(distribution, 1, parties, seats);
}

void SainteLague::init(int* distribution, int ptr_offset, int parties, int seats)
{
    partyN   = parties;
    dist     = distribution;
    offset   = ptr_offset;
    sumSeats = seats;
    sumVotes = 0;
    for (int i = 0; i < parties; i++)
    {
        sumVotes += *(distribution + i * ptr_offset);
    }
}

void SainteLague::getSeatDist(int* results, int ptr_offset)
{
    const double div0 = (double)sumVotes / (double)std::max(sumSeats, partyN);
    vector<std::pair<int, double>> divtable;
    double divisor = 0;

    for (int party = 0; party < partyN; party++)
    {
        divisor = 0.5;
        do
        {
            divtable.push_back(make_pair(party, *(dist + offset * party) / divisor));
            divisor += 1.0;
        } while (divtable.back().second >= div0);
    }
    assert(divtable.size() >= sumSeats);

    std::sort(divtable.begin(), divtable.end(), [](const pair<int,double> &a, const pair<int,double> &b) {return a.second > b.second;});

    for (int party = 0; party < partyN; party++)
    {
        *(results + party * ptr_offset) = 0;
        for (int idx = 0; idx < sumSeats; idx++)
        {
            *(results + party * ptr_offset) += (divtable[idx].first == party);
        }
    }
}


Bundestag::Bundestag(array<StateData, NUM_STATES>& dataFromStates, bool useReform2020, double electoralThreshold, int minNeededDirectMandates) : dataarray(dataFromStates), bUseReform2020(useReform2020), electoralThr(electoralThreshold), minNeededDM(minNeededDirectMandates)
{
    numParties           = calcNumValidParties();
    secondVotes          = (int*)malloc( numParties * sizeof(int) );
    surplusMandates      = (int*)malloc( numParties * sizeof(int) );
    finalSeats           = (int*)malloc( numParties * sizeof(int) );
    compensationMandates = (int*)malloc( numParties * sizeof(int) );
    finalSeatsPerState   = (int**)malloc( numParties * sizeof(int*) );
    for (int s = 0; s < NUM_STATES; s++)
    {
        initialSeatsInStates[s] = (int*)malloc( numParties * sizeof(int) );
    }
    for (int p = 0; p < numParties; p++)
    {
        finalSeatsPerState[p]   = (int*)malloc( NUM_STATES * sizeof(int) );
    }

    this->determineParliament();
    this->calcFinalPartySeatsByState();
}

Bundestag::~Bundestag()
{
    free(secondVotes);
    free(surplusMandates);
    free(finalSeats);
    free(compensationMandates);
    for (int s = 0; s < NUM_STATES; s++)
    {
        free(initialSeatsInStates[s]);
    }
    for (int p = 0; p < numParties; p++)
    {
        free(finalSeatsPerState[p]);
    }
    free(finalSeatsPerState);
}

void Bundestag::evalSurplusMandates()
{
    for (int p = 0; p < numParties; p++)
    {
        surplusMandates[p] = 0;
        for (int s = 0; s < NUM_STATES; s++)
        {
            int dm = dataarray[s].direct_mandates[p];
            if (bUseReform2020)
            {
                int diff = std::max(dm, (int)ceil(0.5 * (dm + initialSeatsInStates[s][p]))) - initialSeatsInStates[s][p];
                surplusMandates[p] += diff;
            }
            else
            {
                int diff = std::max(dm, initialSeatsInStates[s][p]) - initialSeatsInStates[s][p];
                surplusMandates[p] += diff;
            }
        }
        if (bUseReform2020)
        {
            if (surplusMandates[p] < 0) surplusMandates[p] = 0;
        }
        assert( surplusMandates[p] >= 0 );
    }
}

int Bundestag::calcFinalParliamentSize()
{
    int total_seats = 0;
    std::vector<double> divList;
    for (int p = 0; p < numParties; p++)
    {
        finalSeats[p] = bUseReform2020 ? std::max(0, (surplusMandates[p] - 3)) : surplusMandates[p];
        for (int s = 0; s < NUM_STATES; s++)
        {
            finalSeats[p] += initialSeatsInStates[s][p];
        }
        divList.push_back( (double)secondVotes[p] / ((double)finalSeats[p] - 0.5) );
    }
    const double d = *std::min_element(divList.begin(), divList.end());
    divList.clear();

    for (int p = 0; p < numParties; p++)
    {
        finalSeats[p] = (int)round((double)secondVotes[p] / d);
        if (bUseReform2020)
        {
            finalSeats[p] += (surplusMandates[p] - std::max(0, (surplusMandates[p] - 3)));
        }

        total_seats += finalSeats[p];
    }
    return total_seats;
}

int Bundestag::calcNumValidParties()
{
    const int initialNumParties = dataarray[0].first_votes.size();
    validVotes = 0;
    //calc total valid votes
    for (int s = 0; s < NUM_STATES; s++)
    {
        assert( dataarray[s].second_votes.size() == initialNumParties );
        validVotes += dataarray[s].valid_votes[1];
    }
    //clear party list
    for (int p = initialNumParties - 1; p >= 0; p--)
    {
        int scndVotes = 0;
        int wonDMs    = 0;
        bool fulfillNaturalThr = false;
        for (int s = 0; s < NUM_STATES; s++)
        {
            fulfillNaturalThr |= (dataarray[s].second_votes.at(p) * dataarray[s].seats_in_bundestag >= dataarray[s].valid_votes[1]);
            scndVotes += dataarray[s].second_votes.at(p);
            wonDMs    += dataarray[s].direct_mandates.at(p);
        }
        bool fulfillThr = ((double)scndVotes / (double)validVotes >= electoralThr) || (wonDMs >= minNeededDM);
        if (!fulfillThr || !fulfillNaturalThr)
        {
            for (int s = 0; s < NUM_STATES; s++)
            {
                dataarray[s].first_votes.erase(dataarray[s].first_votes.begin() + p);
                dataarray[s].second_votes.erase(dataarray[s].second_votes.begin() + p);
                dataarray[s].direct_mandates.erase(dataarray[s].direct_mandates.begin() + p);
            }
            StateData::party_names.erase(StateData::party_names.begin() + p);
        }
    }
    assert( dataarray[0].first_votes.size() == dataarray[0].second_votes.size() );
    assert( dataarray[0].second_votes.size() == dataarray[0].direct_mandates.size() );
    return dataarray[0].second_votes.size();
}

void Bundestag::determineParliament()
{
    //sum second votes
    for (int p = 0; p < numParties; p++)
    {
        secondVotes[p] = 0;
        for (int s = 0; s < NUM_STATES; s++)
        {
            secondVotes[p] += dataarray[s].second_votes[p];
        }
    }
    for (int s = 0; s < NUM_STATES; s++)
    {
        sl.init(dataarray[s].second_votes.data(), numParties, dataarray[s].seats_in_bundestag);
        sl.getSeatDist(initialSeatsInStates[s]);
    }

    //calc number of surplus mandates
    evalSurplusMandates();

    //calc total number of seats depending on surplus mandates
    totalNumberSeats = calcFinalParliamentSize();

    for (int p = 0; p < numParties; p++)
    {
        compensationMandates[p] = finalSeats[p] - surplusMandates[p];
        for (int s = 0; s < NUM_STATES; s++)
        {
            compensationMandates[p] -= initialSeatsInStates[s][p];
        }
    }
}

void Bundestag::calcFinalPartySeatsByState()
{
    for (int party = 0; party < this->getNumOfParties(); party++)
    {
        int* seatsPerState = finalSeatsPerState[party];
        int votesPerState[NUM_STATES];
        int reduceSeats = 0;
        int actualFinalSeats;
        for (int s = 0; s < NUM_STATES; s++)
        {
            votesPerState[s] = dataarray[s].second_votes[party];
        }
        do
        {
            //sl.init with the array of votes that contains the votes for one <party> in each state
            sl.init(votesPerState, NUM_STATES, finalSeats[party] - reduceSeats);
            sl.getSeatDist(seatsPerState);

            actualFinalSeats = 0;
            for (int s = 0; s < NUM_STATES; s++)
            {
                seatsPerState[s] = std::max(seatsPerState[s], dataarray[s].direct_mandates[party]);
                actualFinalSeats += seatsPerState[s];
            }
            reduceSeats += 1;
        } while ( actualFinalSeats != finalSeats[party] );
    }
}
