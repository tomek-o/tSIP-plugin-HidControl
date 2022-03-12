#ifndef CommThreadH
#define CommThreadH

int CommThreadStart(void);
int CommThreadStop(void);

void UpdateRegistrationState(int state);
void UpdateCallState(int state, const char* display);
void UpdateRing(int state);

#endif // CommThreadH
