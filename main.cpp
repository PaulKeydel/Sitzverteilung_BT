#include "bundestag_lib.h"
#include <fstream>
#include <sstream>

int collectDataFromFile(const string& path, array<StateData, NUM_STATES>& dataarray)
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

    int itemcnt = 0;
    int sum_sonst1, sum_sonst2 = 0;
    int orgNumParties = 0;

    while (std::getline(sstream, record))
    {
        istringstream line(record);

        std::getline(line, record, delimiter);
        //collect further party names
        if (record == "Nr")
        {
            while (std::getline(line, record, delimiter))
            {
                if (!record.empty()) party_names.push_back(record);
                if (!record.empty()) orgNumParties++;
            }
            party_names.erase(party_names.begin(), party_names.begin() + 6);
            orgNumParties -= 6;
        }
        //collect electoral data from each electoral district
        if (record.length() == 3 && std::isdigit(record.at(0)))
        {
            itemcnt = 0;
            StateData* stDataPtr = nullptr;
            int max_first_vote   = 0;
            int won_drct_mandate = 0;

            while (std::getline(line, record, delimiter))
            {
                //get state index
                if (itemcnt == 1) stDataPtr = &dataarray[ std::stoi(record) - 1 ];
                //num of eligible voters
                if (itemcnt == 2) stDataPtr->eligible_voters += std::stoi(record);
                //number of voters in state
                if (itemcnt == 4) stDataPtr->num_voters += std::stoi(record);
                //number of valid first votes
                if (itemcnt == 8) stDataPtr->valid_votes[0] += std::stoi(record);
                //number of valid second votes
                if (itemcnt == 9) stDataPtr->valid_votes[1] += std::stoi(record);
                //first and second votes for parties
                int partyIdx = (itemcnt - 10) / 2;
                if ( itemcnt > 9 && (itemcnt % 2) == 0)
                {
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
                if (itemcnt > 9 && (itemcnt % 2) == 1)
                {
                    stDataPtr->second_votes[partyIdx] += record.empty() ? 0 : std::stoi(record);
                }

                itemcnt++;
            }
            assert( stDataPtr != nullptr );

            sum_sonst1 = 0; sum_sonst2 = 0;
            for (int k = 0; k < orgNumParties; k++)
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
    assert(orgNumParties == party_names.size());

    int sum_direct_mandates = 0;
    for (int i = 0; i < orgNumParties; i++)
    {
        for (int j = 0; j < NUM_STATES; j++) sum_direct_mandates += dataarray[j].direct_mandates[i];
    }
    assert( sum_direct_mandates == NUM_CONSTITUENCIES );

    return orgNumParties;
}


int main(int argc, char *argv[])
{
    string filename(argc > 1 ? argv[1] : "kerg_mod.csv");
  
    array<StateData, NUM_STATES> stData;

    int startingPartiesN = collectDataFromFile(filename, stData);

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

    const int iReformMode = argc > 2 ? atoi(argv[2]) : 2;
    const double dElectThr = argc > 3 ? atof(argv[3]) : 0.05;
    const int iMinNeededDM = argc > 4 ? atoi(argv[4]) : 3;
    Bundestag bt(stData, startingPartiesN, iReformMode, dElectThr, iMinNeededDM, vector<string>{"SSW"});

    //print all parties in parliament
    bt.summaryPrint0();
    //print state summary for each party
    bt.summaryPrint1();

    return 0;
}