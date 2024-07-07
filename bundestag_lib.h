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

extern std::map<int, std::string> stateMap;

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
    void getSeatDist(int* results, int ptr_offset = 1);
};

struct StateData
{
    int eligible_voters = 0;
    int num_voters = 0;
    int valid_votes[2] = {0, 0};
    int seats_in_bundestag = 0;
    //data for parties stored in vectors
    static std::vector<std::string> party_names;
    std::vector<int> first_votes;
    std::vector<int> second_votes;
    std::vector<int> direct_mandates;
};

class Bundestag
{
private:
    SainteLague sl;
    std::array<StateData, NUM_STATES>& dataarray;
    bool bUseReform2020;
    double electoralThr;
    int minNeededDM;
    int* secondVotes;
    int* initialSeatsInStates[NUM_STATES];
    int* surplusMandates;
    int* finalSeats;
    int** finalSeatsPerState;
    int* compensationMandates;
    int totalNumberSeats;
    int validVotes; //total number votes
    int numParties;
    void evalSurplusMandates();
    int calcFinalParliamentSize();
    int calcNumValidParties();
    void determineParliament();
    void calcFinalPartySeatsByState();
public:
    Bundestag(std::array<StateData, NUM_STATES>& dataFromStates, bool useReform2020, double electoralThreshold, int minNeededDirectMandates);
    ~Bundestag();
    int const getDirectMandForState(int state, int party) const {return dataarray[state].direct_mandates[party];}
    int const getNumOfParties() const {return numParties;}
    int const getTotalNumberOfSeats() const {return totalNumberSeats;}
    int const getValidVotes() const {return validVotes;}
    int const getScndVotesForParty(int party) const {return secondVotes[party];}
    int const getSeatAllocation(int party) const {return finalSeats[party];}
    int const getSeatAllocation(int party, int state) const {return finalSeatsPerState[party][state];}
    int const getCompMandForParty(int party) const {return compensationMandates[party];}
    int const getSurplusMandForParty(int party) const {return surplusMandates[party];}
};