#include "bundestag_lib.h"
#include <fstream>
#include <sstream>

void collectDataFromFile(const string& path, array<StateData, NUM_STATES>& dataarray)
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

    while (std::getline(sstream, record))
    {
        istringstream line(record);

        std::getline(line, record, delimiter);
        //collect further party names
        if (record == "Nr")
        {
            while (std::getline(line, record, delimiter))
            {
                if (!record.empty()) StateData::party_names.push_back(record);
            }
            StateData::party_names.erase(StateData::party_names.begin() + 7, StateData::party_names.begin() + 20);
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

void summaryPrint0(const Bundestag& cBT)
{
    cout << std::left;
    for (int p = 0; p < cBT.getNumOfParties(); p++)
    {
        cout << "Seats for " << std::setw(8) << StateData::party_names.at(p) << ": " << std::setw(3) << cBT.getSeatAllocation(p) << "  (ÜM "
          << std::setw(2) << cBT.getSurplusMandForParty(p) << ", AM " << std::setw(2) << cBT.getCompMandForParty(p) << ")   ("
          << std::fixed << std::setprecision(2) << (100.0 * cBT.getScndVotesForParty(p) / cBT.getValidVotes()) << "% votes, "
          << std::fixed << std::setprecision(2) << (100.0 * cBT.getSeatAllocation(p) / cBT.getTotalNumberOfSeats()) << "% seats)" << endl;
    }
    cout << "-------------------------" << endl;
    cout << "Total seats: " << cBT.getTotalNumberOfSeats() << endl;
    cout << "-------------------------" << endl;
}

void summaryPrint1(const Bundestag& cBT)
{
    for (int party = 0; party < cBT.getNumOfParties(); party++)
    {
        cout << StateData::party_names.at(party) << " - Seats per State" << endl;
        cout << "-------------------------" << endl;
        for (int s = 0; s < NUM_STATES; s++)
        {
            cout << std::setw(22) << stateMap.at(s) << " : " << cBT.getSeatAllocation(party, s) << "  (DM " << cBT.getDirectMandForState(s, party) << ")" << endl;
        }
        cout << "-------------------------" << endl;
    }
}


int main(int argc, char *argv[])
{
    string filename(argc > 1 ? argv[1] : "kerg.csv");
  
    array<StateData, NUM_STATES> stData;

    //short names of the first seven parties, rest is taken from file
    StateData::party_names = {"CDU", "SPD", "AfD", "FDP", "Linke", "Gruene", "CSU"};
    collectDataFromFile(filename, stData);

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

    //print all parties in parliament
    summaryPrint0(bt);
    //print state summary for each party
    summaryPrint1(bt);

    return 0;
}