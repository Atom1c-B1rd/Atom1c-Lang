#ifndef INDICATIONS_HPP
#define INDICATIONS_HPP
#include "Decoder.hpp"


enum class IndicationsToken{   

};
class IndicationAction{
    public:
        void StartAction(IndicationsToken* token);
    private:
        void MakeModule();
        void MakeImport();
        void MakeExport();
    //ETC
        



};
typedef struct{
    size_t pos;
    size_t memLoc;
}LocationIndication;
typedef struct{
    IndicationsToken IndicationType;
    IndicationAction Action;
    LocationIndication Loc;
}Indication;

Indication makeIndication(Decoder token);
IndicationsToken* GetActionIndication(TokenDecorder Token);
#endif