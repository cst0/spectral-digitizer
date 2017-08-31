#ifndef commandline_defined
#define commandline_defined

    void do_handshake(void);
    void cli_clear(void);
    void cli_help(void);
    void cli_license(void);
    void cli_helpCommand(void);
    int cli_move(void);
    int cli_movecm(void);
    void cli_scan(void);
    void cli_scancm(void);
    void cli_direction(void);
    void cli_manual(void);
    void cli_gohome(void);
    void cli_sethome(void);
    void cli_goend(void);
    void cli_setend(void);
    void cli_status(void);
    void cli_halt(void);
    int cli_quad(void);
    
#endif