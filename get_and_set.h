#ifndef get_and_set_defined
#define get_and_set_defined

    void doBlinks(void);
    void setMovement(unsigned long long steps);
    void setMovementCM(unsigned long long cm);
    double getPosition(void);
    void doMoves(void);
    bool shouldMove(void);
    void clearQuadRegister(void);
    void setSteps(unsigned long long steps);
    void setDefaultPinState(void);

#endif