#ifndef INPUT_PROMPT_H_
#define INPUT_PROMPT_H_

// Print message, then block on user input. Result is stored in buffer.
int input_string(char *message, char *buffer, int len);

#endif // INPUT_PROMPT_H_