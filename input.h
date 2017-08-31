#ifndef input_defined
#define input_defined

    void clear_tmparray(void);
    void tmparray_from_to_space(char *print, int from);
    unsigned long long parseStringToInt(char toParse[], unsigned int startPoint);
    void echo(char echoChar);
    void clear_user_command(void);
    void putch(unsigned char data);
    void printPrompt(void);
    void startmsg(void);
    int toNumber(char convertThis[]);
    void seperate_strings(char *user_command);
    bool waitForKey(void);
    void badFormatError(void);
    void doInput(char *command);
    void doBackspace(void);
    void handleUART(void);
    
#endif