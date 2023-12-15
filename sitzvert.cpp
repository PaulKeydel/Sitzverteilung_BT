#include "stdio.h"
#include <cassert>
#include <vector>
#include <array>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

using namespace std;

//States of the federal republic
#define SH         0
#define HH         1
#define NI         2
#define HB         3
#define NW         4
#define HE         5
#define RP         6
#define BW         7
#define BY         8
#define SL         9
#define BE         10
#define BB         11
#define MV         12
#define SN         13
#define ST         14
#define TH         15
#define NUM_STATES 16

//electoral districts
#define NUM_CONSTITUENCIES 299

std::map<int, string> stateMap =
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

//names of the first seven parties, rest is taken from file
std::vector<string> partyMap =
{
    "CDU", "SPD", "AfD", "FDP", "Linke", "Gruene", "CSU"
};

class SainteLague
{
    private:
    int* dist;
    int offset;
    int partyN;
    int sumVotes;
    int sumSeats;
    public:
    SainteLague(): dist(nullptr), offset(1), partyN(0), sumVotes(0), sumSeats(0) {}
    void init(int* distribution, int ptr_offset, int parties, int seats);
    void init(int* distribution, int parties, int seats);
    double getDivisor();
    void getSeatDist(int* results, int ptr_offset = 1);
};

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

double SainteLague::getDivisor()
{
    const double div0 = (double)sumVotes / (double)sumSeats;
    vector<double> divtable;
    double divisor = 0;

    for (int party = 0; party < partyN; party++)
    {
        divisor = 0.5;
        do
        {
            divtable.push_back(*(dist + offset * party) / divisor);
            divisor += 1.0;
        } while (divtable.back() >= div0);
    }
    std::sort(divtable.begin(), divtable.end(), std::greater<double>());

    return 0.5 * (divtable.at(sumSeats - 1) + divtable.at(sumSeats));
}

void SainteLague::getSeatDist(int* results, int ptr_offset)
{
    const double div = getDivisor();
    int sum          = 0;
    for (int party = 0; party < partyN; party++)
    {
        *(results + party * ptr_offset) = (int)round((double)*(dist + offset * party) / div);
        sum += *(results + party * ptr_offset);
    }
    assert(sum == sumSeats);
}

struct StateData
{
    int eligible_voters = 0;
    int num_voters = 0;
    int valid_votes[2] = {0, 0};
    int seats_in_bundestag = 0;
    //data for parties stored in vectors
    std::vector<int> first_votes;
    std::vector<int> second_votes;
    std::vector<int> direct_mandates;
    //read the csv data file from Bundeswahlleiter
    static void collectDataFromFile(const string& path, array<StateData, NUM_STATES>& dataarray);
};

void StateData::collectDataFromFile(const string& path, array<StateData, NUM_STATES>& dataarray)
{
    std::stringstream sstream;
    ifstream input_file(path);
    if (!input_file.is_open())
    {
        cerr << "Could not open the file - '" << path << "'" << endl;
        exit(EXIT_FAILURE);
    }
    sstream << input_file.rdbuf();
    
    const char delimiter = ';';
    string record;

    int counter = 0;
    int itemcnt = 0;
    int sum_sonst1, sum_sonst2 = 0;

    while (std::getline(sstream, record))
    {
        istringstream line(record);

        std::getline(line, record, delimiter);
        //collect further party names
        if (record == "Nr")
        {
            while (std::getline(line, record, delimiter))
            {
                if (!record.empty()) partyMap.push_back(record);
            }
            partyMap.erase(partyMap.begin() + 7, partyMap.begin() + 20);
        }
        //collect electoral data from each electoral district
        if (record.length() == 3 && std::isdigit(record.at(0)))
        {
            itemcnt = 0;
            StateData* stDataPtr = nullptr;
            int max_first_vote   = 0;
            int won_drct_mandate = 0;
            bool isFirstConstituencyInState = false;

            while (std::getline(line, record, delimiter))
            {
                //get state index
                if (itemcnt == 1)
                {
                    stDataPtr = &dataarray[ std::stoi(record) - 1 ];
                    isFirstConstituencyInState = (stDataPtr->second_votes.size() == 0);
                }
                //num of eligible voters
                if (itemcnt == 2) stDataPtr->eligible_voters += std::stoi(record);
                //number of voters in state
                if (itemcnt == 6) stDataPtr->num_voters += std::stoi(record);
                //number of valid first votes
                if (itemcnt == 14) stDataPtr->valid_votes[0] += std::stoi(record);
                //number of valid second votes
                if (itemcnt == 16) stDataPtr->valid_votes[1] += std::stoi(record);
                //first and second votes for parties
                int partyIdx = (itemcnt - 18) / 4;
                if ( itemcnt > 16 && ((itemcnt - 18) % 4 == 0))
                {
                    if (isFirstConstituencyInState)
                    {
                        stDataPtr->first_votes.push_back(0);
                        stDataPtr->second_votes.push_back(0);
                        stDataPtr->direct_mandates.push_back(0);
                    }
                    
                    int val = record.empty() ? 0 : std::stoi(record);
                    stDataPtr->first_votes[partyIdx] += val;

                    if (partyIdx == 0)
                    {
                        max_first_vote   = val;
                        won_drct_mandate = partyIdx;
                    }

                    if (val > max_first_vote)
                    {
                        max_first_vote   = val;
                        won_drct_mandate = partyIdx;
                    }
                }
                if (itemcnt > 16 && ((itemcnt - 18) % 4 == 2))
                {
                    stDataPtr->second_votes[partyIdx] += record.empty() ? 0 : std::stoi(record);
                }

                itemcnt++;
            }
            assert( stDataPtr != nullptr );

            assert( stDataPtr->first_votes.size() == stDataPtr->second_votes.size() );
            assert( stDataPtr->second_votes.size() == stDataPtr->direct_mandates.size() );
            
            sum_sonst1 = 0; sum_sonst2 = 0;
            for (int k = 0; k < stDataPtr->first_votes.size(); k++)
            {
                sum_sonst1 += stDataPtr->first_votes[k];
                sum_sonst2 += stDataPtr->second_votes[k];
            }
            assert( sum_sonst1 == stDataPtr->valid_votes[0] );
            assert( sum_sonst2 == stDataPtr->valid_votes[1] );

            assert( max_first_vote != 0 );
            stDataPtr->direct_mandates[won_drct_mandate]++;
        }
    }
    int sum_direct_mandates = 0;
    for (int i = 0; i < dataarray[0].first_votes.size(); i++)
    {
        for (int j = 0; j < NUM_STATES; j++) sum_direct_mandates += dataarray[j].direct_mandates[i];
    }
    assert( sum_direct_mandates == NUM_CONSTITUENCIES );
}

class Bundestag
{
    private:
    SainteLague sl;
    array<StateData, NUM_STATES>& dataarray;
    bool bUseReform2020;
    double electoralThr;
    int minNeededDM;
    int* secondVotes;
    int* initialSeatsInStates[NUM_STATES];
    int* surplusMandates;
    int* finalSeats;
    int* compensationMandates;
    int totalNumberSeats;
    int validVotes; //total number votes
    int numParties;
    void evalSurplusMandates();
    int calcFinalParliamentSize();
    int calcNumValidParties();
    void determineParliament();
    public:
    Bundestag(array<StateData, NUM_STATES>& dataFromStates, bool useReform2020, double electoralThreshold, int minNeededDirectMandates) : dataarray(dataFromStates), bUseReform2020(useReform2020), electoralThr(electoralThreshold), minNeededDM(minNeededDirectMandates)
    {
        numParties           = calcNumValidParties();
        secondVotes          = (int*)malloc( numParties * sizeof(int) );
        surplusMandates      = (int*)malloc( numParties * sizeof(int) );
        finalSeats           = (int*)malloc( numParties * sizeof(int) );
        compensationMandates = (int*)malloc( numParties * sizeof(int) );
        for (int s = 0; s < NUM_STATES; s++)
        {
            initialSeatsInStates[s] = (int*)malloc( numParties * sizeof(int) );
        }

        determineParliament();
    }
    ~Bundestag()
    {
        free(secondVotes);
        free(surplusMandates);
        free(finalSeats);
        free(compensationMandates);
        for (int s = 0; s < NUM_STATES; s++)
        {
            free(initialSeatsInStates[s]);
        }
    }
    void printApportionment();
    void printStateSummaryForParty(int party);
    int const getNumOfParties() {return numParties;}
};

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
            partyMap.erase(partyMap.begin() + p);
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

void Bundestag::printApportionment()
{
    //print summary
    cout << std::left;
    for (int p = 0; p < numParties; p++)
    {
        cout << "Seats for " << std::setw(8) << partyMap.at(p) << ": " << std::setw(3) << finalSeats[p] << "  (ÜM "
          << std::setw(2) << surplusMandates[p] << ", AM " << std::setw(2) << compensationMandates[p] << ")   ("
          << std::fixed << std::setprecision(2) << (100.0 * secondVotes[p] / validVotes) << "% votes, "
          << std::fixed << std::setprecision(2) << (100.0 * finalSeats[p] / totalNumberSeats) << "% seats)" << endl;
    }
    cout << "-------------------------" << endl;
    cout << "Total seats: " << totalNumberSeats << endl;
    cout << "-------------------------" << endl;
}

void Bundestag::printStateSummaryForParty(int party)
{
    assert( (party >=0) && (party < numParties) );
    
    int seatsPerState[NUM_STATES];
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
    
    //print results
    cout << partyMap.at(party) << " - Seats per State" << endl;
    cout << "-------------------------" << endl;
    for (int s = 0; s < NUM_STATES; s++)
    {
        cout << std::setw(22) << stateMap.at(s) << " : " << seatsPerState[s] << "  (DM " << dataarray[s].direct_mandates[party] << ")" << endl;
    }
    cout << "-------------------------" << endl;
}

int main(int argc, char *argv[])
{
    string filename(argc > 1 ? argv[1] : "kerg.csv");
    array<StateData, NUM_STATES> stData;

    StateData::collectDataFromFile(filename, stData);

    //Sitzkontingente je Bundesland
    //https://www.bundeswahlleiter.de/mitteilungen/bundestagswahlen/2021/20210909_btw21-sitzkontingente.html
    stData[SH].seats_in_bundestag = 22;
    stData[MV].seats_in_bundestag = 13;
    stData[HH].seats_in_bundestag = 13;
    stData[NI].seats_in_bundestag = 59;
    stData[HB].seats_in_bundestag = 5;
    stData[BB].seats_in_bundestag = 20;
    stData[ST].seats_in_bundestag = 17;
    stData[BE].seats_in_bundestag = 24;
    stData[NW].seats_in_bundestag = 127;
    stData[SN].seats_in_bundestag = 32;
    stData[HE].seats_in_bundestag = 43;
    stData[TH].seats_in_bundestag = 16;
    stData[RP].seats_in_bundestag = 30;
    stData[BY].seats_in_bundestag = 93;
    stData[BW].seats_in_bundestag = 77;
    stData[SL].seats_in_bundestag = 7;

    const bool bReform2020 = argc > 2 ? (bool)atoi(argv[2]) : true;
    const double dElectThr = argc > 3 ? atof(argv[3]) : 0.05;
    const int iMinNeededDM = argc > 4 ? atoi(argv[4]) : 3;
    Bundestag bt(stData, bReform2020, dElectThr, iMinNeededDM);
    bt.printApportionment();

    for (int p = 0; p < bt.getNumOfParties(); p++)
    {
        bt.printStateSummaryForParty(p);
    }

    return 0;
}