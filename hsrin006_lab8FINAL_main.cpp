#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "spiAVR.h"
#include "serialATmega.h"

#include <stdio.h>
#include <stdlib.h>  // For rand()

#define NUM_TASKS 8 //TODO: Change to the number of tasks being used

//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;


//TODO: Define Periods for each task
const unsigned long TASK1_PERIOD = 100; // joystick button mechanism
const unsigned long TASK2_PERIOD = 100; // screen display (based on jystk button)
const unsigned long TASK3_PERIOD = 50;  // joystick up/down motions + animations
const unsigned long TASK4_PERIOD = 25;  // dino song buzzer
const unsigned long TASK5_PERIOD = 100; // cactus spawn and move
const unsigned long TASK6_PERIOD = 100; // pterodactyl spawn and move
const unsigned long TASK7_PERIOD = 100; // collision check
const unsigned long TASK8_PERIOD = 1; // display and update score on 4D7G

//TODO:Set the GCD Period
const unsigned long GCD_PERIOD = 1;

task tasks[NUM_TASKS]; // declared task array 

// DIRECTIONS METHOD
// Function to check the joystick direction and return an integer
int getJoystickDirection() {
    unsigned int x = ADC_read(2);  // x-axis is on A2
    unsigned int y = ADC_read(3);  // y-axis is on A3

    if (y > 450 && x > 700) {
        return 0; // Up
    }

    else if (y > 450 && x < 300) {
        return 1; // Down
    }

    else if ( (450 < x) && (450 < y)){
        return 2; // Idle 
    }

    else  {
      return 2; 
    }
}

void TimerISR() {
	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime == tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}

//---------------------G L O B A L   V A R I A B L E S-----------------------
unsigned int gameRunning = 0;
unsigned char joystick_pressed = 0;

static int screenWidth = 128; // Screen width (for resetting cactus position)

signed int dino_row_start = 67;
signed int dino_row_end = 77;
signed int dino_col_start = 6;
signed int dino_col_end = 17;
unsigned static int dino_speed = 5;

 static int maxJumpHeight = 42;
 static int minJumpHeight = 64;
signed int jumpDirection = 1;  // 1 for moving up, -1 for moving down, 0 for idle

unsigned long gameTime = 0; //TIMER as GAMERUNNING = 1
unsigned int collisionFlag = 0;

// Variables for Cactus
signed int cactus_row_start = 65;
signed int cactus_row_end = 79;

unsigned int cactus_last_spawn_time = 0;  // Keep track of last spawn time
unsigned int cactus_spawn_interval = 0;   // Randomized spawn interval
signed int cactus_col_start = 0;        // Cactus column start position
signed int cactus_col_end = 0;          // Cactus column end position
unsigned int cactus_speed = 8;            // Cactus speed (movement per tick)

const int max_cactus_spawn_interval = 5000;   // Maximum spawn interval (milliseconds)
const int min_cactus_spawn_interval = 1000;   // Minimum spawn interval (milliseconds)

// Variables for Pterodactyl
signed int pterodactyl_row_start = 55;
signed int pterodactyl_row_end = 62;

unsigned int pterodactyl_last_spawn_time = 0;  // Keep track of last spawn time
unsigned int pterodactyl_spawn_interval = 0;   // Randomized spawn interval
signed int pterodactyl_col_start = 0;        // Pterodactyl column start position
signed int pterodactyl_col_end = 0;          // Pterodactyl column end position
unsigned int pterodactyl_speed = 11;            // Pterodactyl speed (movement per tick)

const int max_pterodactyl_spawn_interval = 11000;   // Maximum spawn interval (milliseconds)
const int min_pterodactyl_spawn_interval = 4000;   // Minimum spawn interval (milliseconds)

//----------------------------------------------------------------------------

void clear_dino()
{
    set_row(41, 79);
    set_column(6, 19);
    send_color(0, 0, 0, 500); // Clear dinosaur
}

void move_dino_up_down()
{
    set_row(dino_row_start, dino_row_start+4);  // Head
    set_column(13, 17);
    send_color(0, 255, 0, 100);

    set_row(dino_row_start + 4, dino_row_start + 9);  // Body
    set_column(9, 15);
    send_color(0, 255, 0, 100);

    set_row(dino_row_start + 9, dino_row_start + 15);  // Left leg
    set_column(10, 12);
    send_color(0, 255, 0, 5);

    set_row(dino_row_start + 9, dino_row_start + 15);  // Right leg
    set_column(13, 15);
    send_color(0, 255, 0, 5);

    set_row(dino_row_start + 7, dino_row_start + 10);  // Tail
    set_column(6, 9);
    send_color(0, 255, 0, 8);
}

//TODO: Create your tick functions for each task

// -------------------------TASK 1: JOYSTICK BUTTON (Period: 100 ms)-------------------------
enum SM1_States {SM1_INIT, JYSTK_OFF_RELEASE, JYSTK_ON_PRESS, JYSTK_ON_RELEASE, JYSTK_OFF_PRESS};

int SM1_tick(int state) {
    // State Transitions
    
    //serial_println(state);  // For debugging state

    switch(state) {
        case SM1_INIT:
            state = JYSTK_OFF_RELEASE;  // Start by waiting for the button to be released
            gameRunning = 0;
            break;

        case JYSTK_OFF_RELEASE:
            if ((GetBit(PINC, 0))) // Button is not pressed
            {  
                state = JYSTK_OFF_RELEASE; // Stay in off release state
            } 
            else 
            {
                state = JYSTK_ON_PRESS; // Transition to on press state
                gameRunning = !gameRunning;  // Toggle the gameRunning flag on button press
            }
            break;

        case JYSTK_ON_PRESS:
            if (!(GetBit(PINC, 0))) {  // Button is still pressed
                state = JYSTK_ON_PRESS;  // Stay in on press state
            } else {
                state = JYSTK_ON_RELEASE;  // Button released after being pressed
            }
            break;

        case JYSTK_ON_RELEASE:
            if (collisionFlag && (!GetBit(PINC, 0))) {  // Button pressed and collisionFlag is true
                collisionFlag = 0;         // Reset collision flag
                serial_println(collisionFlag);
                gameRunning = 0;  
                state = JYSTK_OFF_PRESS;         
            }
            else if ((GetBit(PINC, 0))) {  // Button is still not pressed
                state = JYSTK_ON_RELEASE;  // Stay in on release state
            } else {
                state = JYSTK_OFF_PRESS;   // Button pressed again after releasing
                gameRunning = !gameRunning;  // Toggle the gameRunning flag on release
            }
            break;

        case JYSTK_OFF_PRESS:
            if (!(GetBit(PINC, 0))) {  // Button pressed again
                state = JYSTK_OFF_PRESS; // Stay in off press state
            } else {
                state = JYSTK_OFF_RELEASE; // Return to off release state when button is released
            }
            break;

        default:
            state = SM1_INIT;  // Default state if something unexpected happens
            break;
    }

    // State Actions (additional logic based on state)
    switch(state) {
        case SM1_INIT:
            gameRunning = 0;  // Initialize gameRunning to OFF
            break;

        case JYSTK_OFF_RELEASE:
            break;

        case JYSTK_ON_PRESS:
            //gameRunning = 1;
            break;

        case JYSTK_ON_RELEASE:
            break;

        case JYSTK_OFF_PRESS:
            //gameRunning = 0;
            break;

        default:
            break;
    }

    return state;
}

// -------------------------TASK 2: SCREEN DISPLAY (Period: 100ms)-------------------------
enum SM2_States {SM2_INIT, TITLE_SCREEN, GAME_SCREEN, END_SCREEN};

int SM2_tick(int state) {
    // State Transitions

    switch (state) {
        case SM2_INIT:
            state = TITLE_SCREEN;  // Start with the title screen
            break;

        case TITLE_SCREEN:

            if (gameRunning) {
                black_screen();
                state = GAME_SCREEN;  // Switch to game screen when gameRunning is 1
                
            } else {
                
                state = TITLE_SCREEN;  // Stay on title screen
            }
            break;

        case GAME_SCREEN:
            if (collisionFlag) {
                black_screen();
                state = END_SCREEN;  // Transition to END_SCREEN on collision
            }
            else if (!gameRunning) {
                black_screen();
                state = TITLE_SCREEN;  // Switch back to title screen when gameRunning is 0
                gameTime = 0; // reset game time

            } else {
                gameTime++;
                //serial_println(gameTime);
                state = GAME_SCREEN;  // Stay on game screen
            }
            break;

        case END_SCREEN:
            if (collisionFlag) {
                //gameTime = 0;
                state = END_SCREEN;
            }
            else if (!collisionFlag) {  // If the collision flag is reset, go back to TITLE_SCREEN
                gameTime=0;
                black_screen();
                state = TITLE_SCREEN;
            }
            break;

        default:
            state = SM2_INIT;
            break;
    }

    // State Actions
    switch (state) {
        case SM2_INIT:
            break;

        case TITLE_SCREEN:
            //black_screen();
            displayTitleScreen();  // Display the title screen
            break;

        case GAME_SCREEN:
            //black_screen();
            displayGameScreen();  // Display the game screen
            sun();
            //user_Dino();
            //Pterodactyl();
            //Cactus();
            break;
        
        case END_SCREEN:
            displayGameOverScreen();  // Display game-over screen
            break;

        default:
            break;
    }

    return state;
}

// -------------------------TASK 3: JOYSTICK UP/DOWN MOTIONS (Period: 100ms)-------------------------

enum SM3_States {SM3_INIT, DINO_IDLE, DINO_JUMP, DINO_DUCK};

int SM3_tick(int state) {
  
  // TRANSITIONS
  switch(state)
  {
    case SM3_INIT:
      state = DINO_IDLE; // Intially not moving
      break;

    case DINO_IDLE:
      // Check joystick input to determine next state
      if (getJoystickDirection() == 0) 
      {
        state = DINO_JUMP;
      } else if (getJoystickDirection() == 1) 
      {
        state = DINO_DUCK;
      } 
      break;

    case DINO_JUMP:
      if (getJoystickDirection() == 0) 
      {
        state = DINO_JUMP;
      } 
      else if (getJoystickDirection() == 2)
      {
        // Joystick in idle
        state = DINO_IDLE;
      }  
      break;

    case DINO_DUCK:
      if (getJoystickDirection() == 1) 
      {
        state = DINO_DUCK;
      }  
      else if (getJoystickDirection() == 2)
      {
        // Joystick in idle
        state = DINO_IDLE;
      } 
      break;

    default:
      state = DINO_IDLE;
      break;
  }

  // ACTIONS
  switch(state)
  {
    case SM3_INIT:
      break;

    case DINO_IDLE:

    // clear entire up down range of dino, as well as ducking range
      if(gameRunning)
      {
            clear_dino(); 
      
            dino_row_start = 67;
            dino_row_end = 77;
            dino_col_start = 6;
            dino_col_end = 17;
           
           jumpDirection = 1;
           
           user_Dino();
      } 
       
      break;

    case DINO_JUMP:
        
        if (jumpDirection == 1) {  // Moving up
            if (dino_row_start > maxJumpHeight) 
            {
                set_row(dino_row_start, dino_row_end);
                set_column(dino_col_start, dino_col_end);
                send_color(0, 0, 0, (dino_row_end - dino_row_start + 1) * (dino_col_end - dino_col_start + 1));

                dino_row_start -= dino_speed;
                dino_row_end -= dino_speed;

                move_dino_up_down();
            } 
            else 
            {
                jumpDirection = -1;  // Start moving down when the max height is reached
            }
        } 
        else if (jumpDirection == -1) {  // Moving down
            if (dino_row_start < minJumpHeight) 
            {
                set_row(dino_row_start, dino_row_end);
                set_column(dino_col_start, dino_col_end);
                send_color(0, 0, 0, (dino_row_end - dino_row_start + 1) * (dino_col_end - dino_col_start + 1));

                dino_row_start += dino_speed;
                dino_row_end += dino_speed;

                move_dino_up_down();

            } 
            else 
            {
                jumpDirection = 1;  // Start moving up again when the ground level is reached
            }
        }

      break;

    case DINO_DUCK:
        set_row(dino_row_start, dino_row_end+2);
        set_column(dino_col_start, dino_col_end);
        send_color(0, 0, 0, 150);

        dino_duck_image();
            
      break;

    default:
      break;
  }
  return state;
}

int songIndex = 0;            // Keeps track of the current note
int noteDurationCounter = 0;  // Counter to track ticks for each note
int top = 0;                  // Stores the current Top value

int song_notes[] = {
    NOTE_C5, NOTE_D5, NOTE_C5, NOTE_E5, NOTE_A4, NOTE_A4, NOTE_REST, NOTE_B4, NOTE_B4, NOTE_E5, NOTE_E5, // First line
    NOTE_C5, NOTE_D5, NOTE_C5, NOTE_E5, NOTE_A4, NOTE_A4, NOTE_REST, NOTE_E5, NOTE_B4, NOTE_E5, NOTE_B4, // Second line
    
    NOTE_E5, NOTE_E5, NOTE_A4, NOTE_E5, NOTE_D5, NOTE_REST, NOTE_D5, NOTE_C5, // Opening burst with rests
    NOTE_B4, NOTE_A4, NOTE_C5, NOTE_B4, NOTE_E5, NOTE_REST, NOTE_D5, NOTE_C5, // Main melody with breaks
    NOTE_D5, NOTE_A4, NOTE_REST, NOTE_E5, NOTE_B4, NOTE_A4, NOTE_C5, NOTE_A4, // First loop with rests
    NOTE_REST, NOTE_E5, NOTE_A4, NOTE_D5, NOTE_C5, NOTE_REST, NOTE_A4, NOTE_B4, // Chorus-like section
    NOTE_A4, NOTE_E5, NOTE_B4, NOTE_REST, NOTE_C5, NOTE_B4, NOTE_REST, NOTE_D5, // Bridge and transition
    NOTE_C5, NOTE_A4, NOTE_B4, NOTE_B4, NOTE_E5, NOTE_C5, NOTE_D5, NOTE_B4, // Transition to resolution
    NOTE_A4, NOTE_D5, NOTE_E5, NOTE_REST, NOTE_A4, NOTE_REST, NOTE_C5, NOTE_D5, // Resolution and end with pauses
    NOTE_A4, NOTE_D5, NOTE_E5, NOTE_REST, NOTE_A4, NOTE_REST, NOTE_C5, NOTE_D5, NOTE_D5, NOTE_D5 // Resolution and end with pauses
};

int durations[] = {
    4, 4, 4, 4, 4, 8, 4, 8, 8, 8, 8, // First line durations
    4, 4, 4, 4, 4, 8, 4, 8, 8, 8, 8, // Second line durations
    
    8, 8, 4, 8, 8, 4, 8, 8, // Opening rhythm with rests
    8, 8, 4, 8, 8, 4, 8, 8, // Main melody rhythm
    8, 8, 4, 8, 8, 4, 8, 8, // First loop rhythm with rests
    8, 8, 4, 8, 8, 8, 8, 8, // Chorus-like rhythm with rests
    8, 8, 4, 8, 8, 8, 8, 8, // Bridge rhythm with rests
    8, 8, 4, 8, 8, 8, 8, 8, // Transition rhythm
    8, 4, 4, 8, 8, 4, 8, 8, // Resolution and end with rests
    8, 4, 4, 8, 8, 4, 8, 8, 8, 8 // Resolution and end with rests
};

const int numNotes =  sizeof(song_notes) / sizeof(song_notes[0]);;      // Size of the song array

const unsigned long f_clk = 16000000;  // Microcontroller clock frequency: 16 MHz
const int N = 8;                       // Timer prescaler: divide by 8

// Helper function to calculate Top value
int calculateTop(float f_pwm) {
    return (f_clk / (N * f_pwm)) - 1;
}

void playSong() {
    // Calculate the number of ticks required for the current note duration
    int ticksPerNote = durations[songIndex] * 50 / 100; // Duration * 50ms period, divided by 100ms for ticks.

    if (noteDurationCounter >= ticksPerNote) {  // Check if it's time to move to the next note
        if (songIndex < numNotes) {
            int currentNote = song_notes[songIndex];
            if (currentNote != NOTE_REST) {
                top = calculateTop(currentNote);  // Calculate Top for the current note
                ICR1 = top;                      // Set the timer period
                OCR1A = ICR1 / 2;                // Set the duty cycle (50%)
            } else {
                // If it's a REST, turn off the sound
                OCR1A = 0;
            }
            songIndex++;  // Move to the next note
            noteDurationCounter = 0;  // Reset the counter for the next note
        } else {
            songIndex = 0;  // Loop back to the start of the song
        }
    } else {
        noteDurationCounter++;  // Increment the counter
    }
}

// ---------------------TASK 4: DINO SONG MUSIC (Period: 100 ms)---------------------
enum SM4_States {SM4_INIT, WAIT, SONG_PLAY};

int SM4_tick(int state) {
    // State Transitions
    switch(state) {
        case SM4_INIT:
            state = WAIT; 
            break;

        case WAIT:
            if (gameRunning) 
            {
                state = SONG_PLAY; 
            }
            else if (!gameRunning)
            {
                state = WAIT;
            }
            break;

        case SONG_PLAY:
            if (!gameRunning) 
            {
                state = WAIT; 
            }
            else if (gameRunning)
            {
                state = SONG_PLAY;
            }
            break;

        default:
            state = WAIT; 
            break;
    }

    // State Actions
    switch(state) {
        case WAIT:
            OCR1A = 0;
            songIndex = 0; 
            top = 0;      
            break;

        case SONG_PLAY:
            playSong();
            break;

        default:
            break;
    }
    return state;
}

void Cactus() {
    // CACTUS - Method to draw the cactus at the specified position
    set_row(65, 79);
    set_column(cactus_col_start, cactus_col_end);  
    send_color(0, 100, 0, 50);  // stem

    set_row(70, 73);
    set_column(cactus_col_start - 3, cactus_col_start - 1);  // Left branch
    send_color(0, 100, 0, 9);

    set_row(67, 73);
    set_column(cactus_col_start - 5, cactus_col_start - 3);  // Left branch
    send_color(0, 100, 0, 12);

    set_row(73, 76);
    set_column(cactus_col_start + 2, cactus_col_start + 4);  // Right branch
    send_color(0, 100, 0, 9);

    set_row(69, 74);
    set_column(cactus_col_start + 5, cactus_col_start + 6);  // Right branch
    send_color(0, 100, 0, 15);
}

void clear_cactus_region()
{
    set_row(cactus_row_start, cactus_row_end);
    set_column(dino_col_end+1, screenWidth+1);
    send_color(0, 0, 0, 8000);  // Clear cactus with black

}

// -------------------------TASK 5: CACTUS SPAWNING AND MOVEMENT (Period: 100ms)-------------------------
 enum SM5_States {SM5_INIT, CACTUS_SPAWN, CACTUS_MOVE};  // FSM States for Cactus

int SM5_tick(int state) {
    switch (state) {
        case SM5_INIT:
            // Transition to spawn state
            state = CACTUS_SPAWN;
            break;

        case CACTUS_SPAWN:
            if (gameRunning && (gameTime - cactus_last_spawn_time >= cactus_spawn_interval)) {
                // Initialize spawn position
                cactus_last_spawn_time = gameTime;
                cactus_spawn_interval = rand() % (max_cactus_spawn_interval - min_cactus_spawn_interval + 1) + min_cactus_spawn_interval;
                cactus_col_start = screenWidth;
                cactus_col_end = cactus_col_start + 2;

             

                state = CACTUS_MOVE;
            }
            break;

        case CACTUS_MOVE:
            if (gameRunning) {
                if (cactus_col_start > 0) {
                    // Move cactus and redraw
                    clear_cactus_region();
                    cactus_col_start -= cactus_speed;
                    cactus_col_end -= cactus_speed;
                    Cactus();

             
                } else {
                    // Reset position when out of screen
                    clear_cactus_region();
                    cactus_col_start = screenWidth;
                    cactus_col_end = cactus_col_start + 2; 
                }
            }
            break;

        default:
            state = SM5_INIT;
            break;
    }
    return state;
}



void Pterodactyl()
{
    // PTERODACTYL

    // Top point
    set_row(49, 50);  // Moved 8 rows higher
    set_column(pterodactyl_col_start+2, pterodactyl_col_start+3);  // Moved 20 columns to the right
    send_color(255, 165, 0, 100);  // head

    // Middle row
    set_row(50, 51);  // Moved 8 rows higher
    set_column(pterodactyl_col_start+1, pterodactyl_col_start+4);  // Moved 20 columns to the right
    send_color(255, 165, 0, 100);  // head

    // Bottom row
    set_row(51, 52);  // Moved 8 rows higher
    set_column(pterodactyl_col_start, pterodactyl_col_start+5);  // Moved 20 columns to the right
    send_color(255, 165, 0, 105);  // head

    set_row(52, 54);  // Moved 8 rows higher
    set_column(pterodactyl_col_start+4, pterodactyl_col_start+9);  // Moved 20 columns to the right
    send_color(255, 165, 0, 100);  // Body

    set_row(47, 48);  // Moved 8 rows higher
    set_column(pterodactyl_col_start+6, pterodactyl_col_start+7);  // Moved 20 columns to the right
    send_color(255, 165, 0, 100);  // Wing

    set_row(48, 49);  // Moved 8 rows higher
    set_column(pterodactyl_col_start+6, pterodactyl_col_start+8);  // Moved 20 columns to the right
    send_color(255, 165, 0, 100);  // Wing

    set_row(49, 50);  // Moved 8 rows higher
    set_column(pterodactyl_col_start+6, pterodactyl_col_start+9);  // Moved 20 columns to the right
    send_color(255, 165, 0, 100);  // Wing

    set_row(50, 51);  // Moved 8 rows higher
    set_column(pterodactyl_col_start+6, pterodactyl_col_start+10);  // Moved 20 columns to the right
    send_color(255, 165, 0, 100);  // Wing

    set_row(51, 52);  // Moved 8 rows higher
    set_column(pterodactyl_col_start+9, pterodactyl_col_end);  // Moved 20 columns to the right
    send_color(255, 165, 0, 10);  // Tail
}

// void clear_pterodactyl_region()
// {
//     set_row(pterodactyl_row_start, pterodactyl_row_end);  // Match pterodactyl's vertical range
//     set_column(pterodactyl_col_start, pterodactyl_col_end);  // Match current horizontal range
//     send_color(0, 0, 0, (pterodactyl_col_end - pterodactyl_col_start + 1) * 7);  // Clear accurately
// }
void clear_pterodactyl_region()
{
    // Correctly clear based on the pterodactyl's actual start and end columns
    set_row(55, 62);  // Rows to clear for pterodactyl (adjust as needed)
    set_column(pterodactyl_col_start, pterodactyl_col_end);  // Correct columns based on pterodactyl's position
    send_color(0, 0, 0, (pterodactyl_col_end - pterodactyl_col_start + 1) * 8);  // Clear area with black
}


// -------------------------TASK 6: PTERODACTYL SPAWNING AND MOVEMENT (Period: 100ms)-------------------------
enum SM6_States {SM6_INIT, PTERODACTYL_SPAWN, PTERODACTYL_MOVE};  // FSM States for Pterodactyl

int SM6_tick(int state) {
    switch (state) {
        case SM6_INIT:
            // Transition to spawn state
            state = PTERODACTYL_SPAWN;
            break;

        case PTERODACTYL_SPAWN:
            if (gameRunning && (gameTime - pterodactyl_last_spawn_time >= pterodactyl_spawn_interval)) {
                // Initialize spawn position
                pterodactyl_last_spawn_time = gameTime;
                pterodactyl_spawn_interval = rand() % (max_pterodactyl_spawn_interval - min_pterodactyl_spawn_interval + 1) + min_pterodactyl_spawn_interval;
                pterodactyl_col_start = screenWidth;
                pterodactyl_col_end = pterodactyl_col_start + 2;

             

                state = PTERODACTYL_MOVE;
            }
            break;

        case PTERODACTYL_MOVE:
            if (gameRunning) {
                if (pterodactyl_col_start > 0) {
                    // Move pterodactyl and redraw
                    clear_pterodactyl_region();
                    pterodactyl_col_start -= pterodactyl_speed;
                    pterodactyl_col_end -= pterodactyl_speed;
                    Pterodactyl();

                    
                } else {
                    // Reset position when out of screen
                    clear_pterodactyl_region();
                    pterodactyl_col_start = screenWidth;
                    pterodactyl_col_end = pterodactyl_col_start + 2;

                    
                }
            }
            break;

        default:
            state = SM6_INIT;
            break;
    }
    return state;
}



// -------------------------TASK 7: DINO COLLISION DETECTION (Period: 50ms)-------------------------
enum SM7_States {SM7_INIT, CHECK_COLLISION};  

int SM7_tick(int state) {
    // State Transitions
    int cactus_collision = 0;
    int ptero_collision = 0;
    switch (state) {
        case SM7_INIT:
            state = CHECK_COLLISION;  // Always stay in the same state
            break;

        case CHECK_COLLISION:
            state = CHECK_COLLISION; 
            break;

        default:
            state = SM7_INIT;
            break;
    }

    // State Actions
    switch (state) {
        case SM7_INIT:
            break;

        case CHECK_COLLISION:
            if (gameRunning) {
                // Check for collision with Cactus
               
                //cactus_collision = (dino_row_start < cactus_row_end && dino_col_end > cactus_col_start);
                cactus_collision = (dino_col_end > cactus_col_start && dino_col_start < cactus_col_end) &&
                   (dino_row_end > cactus_row_start && dino_row_start < cactus_row_end);

                // Check for collision with Pterodactyl
                //ptero_collision = (dino_row_end < pterodactyl_row_start && dino_col_end > pterodactyl_col_start);
                ptero_collision = (dino_col_end > pterodactyl_col_start && dino_col_start < pterodactyl_col_end) &&
                  (dino_row_end > pterodactyl_row_start && dino_row_start < pterodactyl_row_end);


                // If any collision is detected
                if (cactus_collision || ptero_collision) {
                    gameRunning = 0;  // Stop the game
                    cactus_col_end = 0;
                    cactus_col_start = 0;
                    pterodactyl_col_end = 0;
                    pterodactyl_col_start = 0;
                    //int hit = 1;
                    //serial_println(hit);
                    // --------------ADD FLAG TO INDICATE GAME OVER---------------
                    collisionFlag = 1;
                    
                }
                else if (!(cactus_collision || ptero_collision)) {
                    set_row(40, 79);
                    set_column(0, 5);
                    send_color(0, 0, 0, 500); // Clear
                }
            }
            break;

        default:
            break;
    }
    return state;
}

// -------------------------TASK 8: Update and Display Score (Period: 1000 ms)-------------------------
enum SM8_States {SM8_INIT, ONES_DIGIT, TENS_DIGIT, HUNDS_DIGIT, THOUS_DIGIT};

int SM8_tick(int state) {
  // State Transitions
  //serial_println(state);
  switch(state) {
    case SM8_INIT:
      state = ONES_DIGIT; 
      break;

    case ONES_DIGIT:
      state = TENS_DIGIT; // Move to the tens digit
      break;

    case TENS_DIGIT:
      state = HUNDS_DIGIT; // Move to the hundreds digit
      break;

    case HUNDS_DIGIT:
      state = THOUS_DIGIT; // Move to the thousands digit
      break;

    case THOUS_DIGIT:
      state = ONES_DIGIT; // Loop back to the ones digit
      break;

    default:
      state = SM8_INIT; 
      break;
  }

  // State Actions
  switch(state) {
    case SM8_INIT:
      break;

    case ONES_DIGIT:
      // Activate D1 (PINC4)
      PORTD = SetBit(PORTD, 5, 1);
      PORTD = SetBit(PORTD, 6, 1);
      PORTC = SetBit(PORTC, 4, 1);
      PORTD = SetBit(PORTD, 7, 1);
      /// Extract ones place
    displayNumber((gameTime) % 10);
      PORTD = SetBit(PORTD, 7, 0);
      break;

    case TENS_DIGIT:
      // Activate D2 (PIND5)
      PORTC = SetBit(PORTC, 4, 1);
      PORTD = SetBit(PORTD, 5, 1);
      PORTD = SetBit(PORTD, 7, 1);
      PORTD = SetBit(PORTD, 6, 1);
      // Extract tens place
     displayNumber((gameTime)/10);
      // Deactivate other digits
      PORTD = SetBit(PORTD, 6, 0);
      break;

    case HUNDS_DIGIT:
      // Activate D3 (PIND6)
      PORTC = SetBit(PORTC, 4, 1);
      PORTD = SetBit(PORTD, 6, 1);
      PORTD = SetBit(PORTD, 7, 1);
      PORTD = SetBit(PORTD, 5, 1);
      // Extract hundreds place
    displayNumber((gameTime)/100);
      // Deactivate other digits
      PORTD = SetBit(PORTD, 5, 0);
      break;

    case THOUS_DIGIT:
      // Activate D4 (PIND7)
      PORTD = SetBit(PORTD, 7, 1);
      PORTD = SetBit(PORTD, 5, 1);
      PORTD = SetBit(PORTD, 6, 1);
      PORTC = SetBit(PORTC, 4, 1);
      // Extract thousands place
     displayNumber((gameTime)/1000);
      // Deactivate other digits
      PORTC = SetBit(PORTC, 4, 0);
      break;

    default:
      break;
  }

  return state;
}


int main(void) {
    ADC_init();   // initializes ADC
    sonar_init(); // initializes sonar
    
    serial_init(9600);
    

    //TODO: initialize all your inputs and ouputs
    DDRC = 0x10;  PORTC = 0xFF;
    DDRD = 0xFF;  PORTD = 0x00;
    DDRB = 0x2F;  PORTB = 0x00;

    SPI_INIT();
    HardwareReset();
    ST7735_init();
    black_screen(); 

    //TODO: Initialize the servo timer/pwm(timer1)

    TCCR1A |= (1 << WGM11) | (1 << COM1A1); //COM1A1 sets it to channel A
    TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11); //CS11 sets the prescaler to be 8
    //WGM11, WGM12, WGM13 set timer to fast pwm mode

    //ICR1 = 39999; // 20ms pwm period
    OCR1A = 0;

    //TODO: Initialize tasks here
     // TASK 1
    tasks[0].period = TASK1_PERIOD;
    tasks[0].state = SM1_INIT;
    tasks[0].elapsedTime = TASK1_PERIOD;
    tasks[0].TickFct = &SM1_tick;

    // TASK 2
    tasks[1].period = TASK2_PERIOD;
    tasks[1].state = SM2_INIT;
    tasks[1].elapsedTime = TASK2_PERIOD;
    tasks[1].TickFct = &SM2_tick;

    // TASK 3
    tasks[2].period = TASK3_PERIOD;
    tasks[2].state = SM3_INIT;
    tasks[2].elapsedTime = TASK3_PERIOD;
    tasks[2].TickFct = &SM3_tick;

    // TASK 4
    tasks[3].period = TASK4_PERIOD;
    tasks[3].state = SM4_INIT;
    tasks[3].elapsedTime = TASK4_PERIOD;
    tasks[3].TickFct = &SM4_tick;

    // TASK 5
    tasks[4].period = TASK5_PERIOD;
    tasks[4].state = SM5_INIT;
    tasks[4].elapsedTime = TASK5_PERIOD;
    tasks[4].TickFct = &SM5_tick;

    // TASK 6
    tasks[5].period = TASK6_PERIOD;
    tasks[5].state = SM6_INIT;
    tasks[5].elapsedTime = TASK6_PERIOD;
    tasks[5].TickFct = &SM6_tick;

    // TASK 7
    tasks[6].period = TASK7_PERIOD;
    tasks[6].state = SM7_INIT;
    tasks[6].elapsedTime = TASK7_PERIOD;
    tasks[6].TickFct = &SM7_tick;

    // TASK 8
    tasks[7].period = TASK8_PERIOD;
    tasks[7].state = SM8_INIT;
    tasks[7].elapsedTime = TASK8_PERIOD;
    tasks[7].TickFct = &SM8_tick;

    TimerSet(GCD_PERIOD);
    TimerOn();
    
    

    while (1) {}
    

    return 0;
}