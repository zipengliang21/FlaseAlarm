#pragma once
// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>



class ConversationState
{

public:
    enum class CONVERSATION_STATE {
        NO_CONVERSATION_YET,
        DURING_CONVERSATION,
        NO_CRIME_DETECTED,
        CRIME_DETECTED
    };

    ConversationState() {
        state = CONVERSATION_STATE::NO_CONVERSATION_YET;
    }

    CONVERSATION_STATE getState() {
        return this->state;
    }

    void setState(CONVERSATION_STATE s) {
        state = s;
    }

private:
    CONVERSATION_STATE state;

};
