#include "stdio.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <array>
#include <math.h>
#include <map>
#include <iomanip>

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

//map between federal state names and their key value
extern std::map<int, std::string> stateMap;
//short names for all parties involved in election
extern std::vector<std::string> party_names;

class SainteLague
{
private:
    int* dist;
    int offset;
    int partyN;
    int sumVotes;
public:
    SainteLague(): dist(nullptr), offset(1), partyN(0), sumVotes(0) {}
    void init(int totalNumParties, int* distribution, int ptr_offset = 1);
    void getSeatDist(int totalSeats, int* results, int ptr_offset = 1);
};

struct StateData
{
    int eligible_voters = 0;
    int num_voters = 0;
    int valid_votes[2] = {0, 0};
    int seats_in_bundestag = 0;
    std::vector<int> first_votes;
    std::vector<int> second_votes;
    std::vector<int> direct_mandates;
};

struct ParlGroupData
{
    int surplusMandates = 0;
    int finalSeats = 0;
    int finalSeatsPerState[NUM_STATES] = {0};
    int compensationMandates = 0;
};

class Bundestag
{
private:
    SainteLague sl;
    std::array<StateData, NUM_STATES>& dataarray;
    std::vector<ParlGroupData> datapg;
    bool bUseReform2020;
    int i1reform2024_2reform2020_3before;
    double electoralThr;
    int minNeededDM;
    int* secondVotes;
    int* initialSeatsInStates[NUM_STATES];
    int totalNumberSeats;
    int validVotes; //total number votes
    int numParties;
    void evalSurplusMandates();
    int calcFinalParliamentSize();
    int calcNumValidParties();
    void calcFinalPartySeatsByState();
public:
    Bundestag(std::array<StateData, NUM_STATES>& dataFromStates, int i1reform2024_2reform2020_3before, double electoralThreshold, int minNeededDirectMandates);
    ~Bundestag();
    const ParlGroupData& Fraktion(int party) const {return datapg.at(party);}
    ParlGroupData& Fraktion(int party) {return datapg.at(party);}
    void summaryPrint0();
    void summaryPrint1();
    int const getDirectMandForState(int state, int party) const {return dataarray[state].direct_mandates[party];}
    int const getNumOfParties() const {return numParties;}
    int const getTotalNumberOfSeats() const {return totalNumberSeats;}
    int const getValidVotes() const {return validVotes;}
    int const getScndVotesForParty(int party) const {return secondVotes[party];}
};