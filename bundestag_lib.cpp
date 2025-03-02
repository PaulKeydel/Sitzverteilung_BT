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
vector<std::string> party_names = {};

const int ParlGroupData::SIZE = sizeof(ParlGroupData) / sizeof(int);
const int StateData::SIZE     = 3 * MAX_NUM_PARTIES + 5;


void SainteLague::init(int totalNumParties, int* distribution, int ptr_offset)
{
    partyN   = totalNumParties;
    dist     = distribution;
    offset   = ptr_offset;
    sumVotes = 0;
    for (int i = 0; i < totalNumParties; i++)
    {
        sumVotes += *(distribution + i * ptr_offset);
    }
}

void SainteLague::getSeatDist(int totalSeats, int* results, int ptr_offset)
{
    const double div0 = (double)sumVotes / (double)std::max(totalSeats, partyN);
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
    assert(divtable.size() >= totalSeats);

    std::sort(divtable.begin(), divtable.end(), [](const pair<int,double> &a, const pair<int,double> &b) {return a.second > b.second;});

    for (int party = 0; party < partyN; party++)
    {
        *(results + party * ptr_offset) = 0;
        for (int idx = 0; idx < totalSeats; idx++)
        {
            *(results + party * ptr_offset) += (divtable[idx].first == party);
        }
    }
}


Bundestag::Bundestag(array<StateData, NUM_STATES>& dataFromStates, int numPartiesAtStart, int i1reform2024_2reform2020_3before, double electoralThreshold, int minNeededDirectMandates, std::vector<std::string>&& listNatMinPar):
    stateData(dataFromStates),
    i1reform2024_2reform2020_3before(i1reform2024_2reform2020_3before),
    electoralThr(electoralThreshold),
    minNeededDM(minNeededDirectMandates),
    initialNumParties(numPartiesAtStart),
    natMinParties(listNatMinPar)
{
    numParties = calcNumValidParties();
    for (int s = 0; s < NUM_STATES; s++)
    {
        initialSeatsInStates[s] = (int*)malloc( numParties * sizeof(int) );
    }
    parlGrData.resize(numParties);

    //sum second votes for each party over all federal states
    for (int p = 0; p < numParties; p++)
    {
        Fraktion(p).secondVotes = 0;
        for (int s = 0; s < NUM_STATES; s++)
        {
            Fraktion(p).secondVotes += Bundesland(s).second_votes[p];
        }
    }
    //do the calculations depending on the chosen Wahlrechtsreform
    if (i1reform2024_2reform2020_3before == 1)
    {
        totalNumberSeats = 630;
        int votesPerState[NUM_STATES];
        //Oberverteilung nach https://www.bundeswahlleiterin.de/dam/jcr/05f98632-634d-4582-8507-ab3267d66c01/bwg2025_sitzberechnung_erg2021.pdf
        sl.init(numParties, &Fraktion(0).secondVotes, ParlGroupData::SIZE);
        sl.getSeatDist(totalNumberSeats, &Fraktion(0).finalSeats, ParlGroupData::SIZE);
        //Unterverteilung
        for (int party = 0; party < numParties; party++)
        {
            for (int s = 0; s < NUM_STATES; s++)
            {
                votesPerState[s] = Bundesland(s).second_votes[party];
            }
            sl.init(NUM_STATES, votesPerState);
            sl.getSeatDist(Fraktion(party).finalSeats, Fraktion(party).finalSeatsPerState);
        }
    }
    else
    {
        bUseReform2020 = (i1reform2024_2reform2020_3before == 2);
        //calc party specific seat allocation in a state <s>
        for (int s = 0; s < NUM_STATES; s++)
        {
            sl.init(numParties, Bundesland(s).second_votes);
            sl.getSeatDist(Bundesland(s).seats_in_bundestag, initialSeatsInStates[s]);
        }

        //calc number of surplus mandates
        evalSurplusMandates();

        //calc total number of seats depending on surplus mandates
        totalNumberSeats = calcFinalParliamentSize();

        calcFinalPartySeatsByState();
    }
}

Bundestag::~Bundestag()
{
    for (int s = 0; s < NUM_STATES; s++)
    {
        free(initialSeatsInStates[s]);
    }
}

void Bundestag::evalSurplusMandates()
{
    for (int p = 0; p < numParties; p++)
    {
        Fraktion(p).surplusMandates = 0;
        for (int s = 0; s < NUM_STATES; s++)
        {
            int dm = Bundesland(s).direct_mandates[p];
            if (bUseReform2020)
            {
                int diff = std::max(dm, (int)ceil(0.5 * (dm + initialSeatsInStates[s][p]))) - initialSeatsInStates[s][p];
                Fraktion(p).surplusMandates += diff;
            }
            else
            {
                int diff = std::max(dm, initialSeatsInStates[s][p]) - initialSeatsInStates[s][p];
                Fraktion(p).surplusMandates += diff;
            }
        }
        if (bUseReform2020)
        {
            if (Fraktion(p).surplusMandates < 0) Fraktion(p).surplusMandates = 0;
        }
        assert( Fraktion(p).surplusMandates >= 0 );
    }
}

int Bundestag::calcFinalParliamentSize()
{
    int total_seats = 0;
    std::vector<double> divList;
    for (int p = 0; p < numParties; p++)
    {
        Fraktion(p).finalSeats = bUseReform2020 ? std::max(0, (Fraktion(p).surplusMandates - 3)) : Fraktion(p).surplusMandates;
        for (int s = 0; s < NUM_STATES; s++)
        {
            Fraktion(p).finalSeats += initialSeatsInStates[s][p];
        }
        divList.push_back( (double)Fraktion(p).secondVotes / ((double)Fraktion(p).finalSeats - 0.5) );
    }
    const double d = *std::min_element(divList.begin(), divList.end());
    for (int p = 0; p < numParties; p++)
    {
        total_seats += (int)round((double)Fraktion(p).secondVotes / d);
    }
    divList.clear();

    sl.init(numParties, &Fraktion(0).secondVotes, ParlGroupData::SIZE);
    sl.getSeatDist(total_seats, &Fraktion(0).finalSeats, ParlGroupData::SIZE);

    for (int p = 0; p < numParties; p++)
    {
        if (bUseReform2020)
        {
            int iadd = (Fraktion(p).surplusMandates - std::max(0, (Fraktion(p).surplusMandates - 3)));
            Fraktion(p).finalSeats += iadd;
            total_seats += iadd;
        }

        Fraktion(p).compensationMandates = Fraktion(p).finalSeats - Fraktion(p).surplusMandates;
        for (int s = 0; s < NUM_STATES; s++)
        {
            Fraktion(p).compensationMandates -= initialSeatsInStates[s][p];
        }
    }
    return total_seats;
}

int Bundestag::calcNumValidParties()
{
    int finNumParties = initialNumParties;
    validVotes = 0;
    //calc total valid votes
    for (int s = 0; s < NUM_STATES; s++)
    {
        validVotes += Bundesland(s).valid_votes[1];
    }
    //clear party list
    for (int p = initialNumParties - 1; p >= 0; p--)
    {
        int scndVotes = 0;
        int wonDMs    = 0;
        bool fulfillNaturalThr = false;
        for (int s = 0; s < NUM_STATES; s++)
        {
            fulfillNaturalThr |= (Bundesland(s).second_votes[p] * Bundesland(s).seats_in_bundestag >= Bundesland(s).valid_votes[1]);
            scndVotes += Bundesland(s).second_votes[p];
            wonDMs    += Bundesland(s).direct_mandates[p];
        }
        const bool fulfillThr    = ((double)scndVotes / (double)validVotes >= electoralThr) || (wonDMs >= minNeededDM);
        const bool isNatMinParty = (find(natMinParties.begin(), natMinParties.end(), party_names.at(p)) != natMinParties.end());
        if ((!fulfillThr || !fulfillNaturalThr) && !isNatMinParty)
        {
            for (int s = 0; s < NUM_STATES; s++)
            {
                //erase current element p
                copy(Bundesland(s).first_votes + p + 1, Bundesland(s).first_votes + MAX_NUM_PARTIES, Bundesland(s).first_votes + p);
                copy(Bundesland(s).second_votes + p + 1, Bundesland(s).second_votes + MAX_NUM_PARTIES, Bundesland(s).second_votes + p);
                copy(Bundesland(s).direct_mandates + p + 1, Bundesland(s).direct_mandates + MAX_NUM_PARTIES, Bundesland(s).direct_mandates + p);
            }
            finNumParties--;
            party_names.erase(party_names.begin() + p);
        }
    }
    return finNumParties;
}

void Bundestag::calcFinalPartySeatsByState()
{
    for (int party = 0; party < this->getNumOfParties(); party++)
    {
        int reduceSeats = 0;
        int actualFinalSeats;

        sl.init(NUM_STATES, &Bundesland(0).second_votes[party], StateData::SIZE);
        do
        {
            sl.getSeatDist(Fraktion(party).finalSeats - reduceSeats, Fraktion(party).finalSeatsPerState);

            actualFinalSeats = 0;
            for (int s = 0; s < NUM_STATES; s++)
            {
                Fraktion(party).finalSeatsPerState[s] = std::max(Fraktion(party).finalSeatsPerState[s], Bundesland(s).direct_mandates[party]);
                actualFinalSeats += Fraktion(party).finalSeatsPerState[s];
            }
            reduceSeats += 1;
        } while ( actualFinalSeats != Fraktion(party).finalSeats );
    }
}

void Bundestag::summaryPrint0()
{
    cout << std::left;
    for (int p = 0; p < getNumOfParties(); p++)
    {
        cout << "Seats for " << std::setw(8) << party_names.at(p) << ": " << std::setw(3) << Fraktion(p).finalSeats << "  (ÜM "
          << std::setw(2) << Fraktion(p).surplusMandates << ", AM " << std::setw(2) << Fraktion(p).compensationMandates << ")   ("
          << std::fixed << std::setprecision(2) << (100.0 * Fraktion(p).secondVotes / getValidVotes()) << "% votes, "
          << std::fixed << std::setprecision(2) << (100.0 * Fraktion(p).finalSeats / getTotalNumberOfSeats()) << "% seats)" << endl;
    }
    cout << "-------------------------" << endl;
    cout << "Total seats: " << getTotalNumberOfSeats() << endl;
    cout << "-------------------------" << endl;
}

void Bundestag::summaryPrint1()
{
    for (int party = 0; party < getNumOfParties(); party++)
    {
        cout << party_names.at(party) << " - Seats per State" << endl;
        cout << "-------------------------" << endl;
        for (int s = 0; s < NUM_STATES; s++)
        {
            cout << std::setw(22) << stateMap.at(s) << " : " << Fraktion(party).finalSeatsPerState[s] << "  (DM " <<  Bundesland(s).direct_mandates[party] << ")";
            int iOverhang = Bundesland(s).direct_mandates[party] - Fraktion(party).finalSeatsPerState[s];
            if (i1reform2024_2reform2020_3before != 1) assert(iOverhang <= 0);
            if (iOverhang > 0) cout << "   <-- " << iOverhang << " winner/s not represented in Bundestag";
            cout << endl;
        }
        cout << "-------------------------" << endl;
    }
}