#include "ripes_system.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

#ifndef RIPES_IO_HEADER
#define RIPES_IO_HEADER
// *****************************************************************************
// * SWITCHES_0
// *****************************************************************************
#define SWITCHES_0_BASE	(0xf0000000)
#define SWITCHES_0_SIZE	(0x4)
#define SWITCHES_0_N	(0x8)


// *****************************************************************************
// * LED_MATRIX_0
// *****************************************************************************
#define LED_MATRIX_0_BASE	(0xf0000004)
#define LED_MATRIX_0_SIZE	(0x1900)
#define LED_MATRIX_0_WIDTH	(0x28)
#define LED_MATRIX_0_HEIGHT	(0x28)


// *****************************************************************************
// * D_PAD_0
// *****************************************************************************
#define D_PAD_0_BASE	(0xf0001904)
#define D_PAD_0_SIZE	(0x10)
#define D_PAD_0_UP_OFFSET	(0x0)
#define D_PAD_0_UP	(0xf0001904)
#define D_PAD_0_DOWN_OFFSET	(0x4)
#define D_PAD_0_DOWN	(0xf0001908)
#define D_PAD_0_LEFT_OFFSET	(0x8)
#define D_PAD_0_LEFT	(0xf000190c)
#define D_PAD_0_RIGHT_OFFSET	(0xc)
#define D_PAD_0_RIGHT	(0xf0001910)


#endif // RIPES_IO_HEADER

//Variables globales
#define RED 0xff0000 //Snake
#define BLUE 0x0000ff //Snake's head
#define GREEN 0x00ff00 //apples
#define BLACK 0x000000 //background = leds off
#define INITIAL_SNAKE_SIZE 4 //Tamaño inicial de snake = 4
//Para definir el estado de snake
#define SNAKE_ALIVE 1
#define SNAKE_DEAD 2
#define SNAKE_WIN 3

//Parseos para Matriz de LEDs y D-Pad
volatile unsigned int *led_base = (int *) LED_MATRIX_0_BASE;
volatile unsigned int *btn_up = (int *) D_PAD_0_UP;
volatile unsigned int *btn_down = (int *) D_PAD_0_DOWN;
volatile unsigned int *btn_right = (int *) D_PAD_0_RIGHT;
volatile unsigned int *btn_left = (int *) D_PAD_0_LEFT;
volatile unsigned int *switch_0 = (int *) SWITCHES_0_BASE;

//Configuraciones de la Matriz de LEDs
const int led_width = LED_MATRIX_0_WIDTH;
const int led_height = LED_MATRIX_0_HEIGHT;
const int led_size = LED_MATRIX_0_SIZE;

//Offsets
const int right_offset = 1;
const int left_offset = - 1;
const int up_offset = -led_width;
const int down_offset = led_width;

int rand_time = 50;
int tail_index;

//Apuntadores a la snake
int *snake_parts[LED_MATRIX_0_WIDTH * LED_MATRIX_0_HEIGHT];

//Estado del juego
int game_state;
typedef enum _bool {false, true} boolean; //Para ver el estado

//Crear el marco para definir cuando la snake choca
void init_tablero(){
     int *init_ptr = led_base;
    for (int i = 0; i < led_height; i++){
        *init_ptr = BLUE;
        *(init_ptr + led_width - 1) = BLUE;
        init_ptr += led_width;
    }
    
    init_ptr = led_base;
    for (int i = 0; i < led_width; i++){
        *(init_ptr + i) = BLUE;
        *(init_ptr + (led_height * led_width - led_width) + i) = BLUE;
    }
}

//Crea la serpiente
void init_snake(){
    tail_index = INITIAL_SNAKE_SIZE - 1; //Tamaño del cuerpo
    //Definimos la cabeza de la serpiente
    int *head = led_base + (((led_width) * (led_height >> 1)) - (led_width >> 1));
    snake_parts[0] = head;
    //Ponemos el cuerpo y la cola de la serpiente
    for (int i = 1; i <= tail_index; i++){
        snake_parts[i] = snake_parts[i -1] - 1;
    }
}

//Generar manzanas
void create_apple(){
    boolean is_valid = false;
    rand_time += 5;
    
    while (!is_valid){
        srand(rand_time);
        //Coordenadas
        int rand_x = rand() % (led_width - 2);
        int rand_y = rand() % (led_height - 2);
        //Calcular pos
        int *rand_pos = led_base + (rand_y * led_width + rand_x);
        
        //Poner manzana
        //Checa si no es borde o snake
        if (*rand_pos != BLUE && *rand_pos != RED){
            is_valid = true;
            *rand_pos = GREEN;
        }
    }
}

void move_snake (int offset){
    int *old_part = snake_parts[0];
    snake_parts[0] += offset;
    int *head_ptr = snake_parts[0];
    
    //Revisar si la snake choca contra una pared o ella misma
    if (*head_ptr == BLUE || *head_ptr == RED) {
        game_state = SNAKE_DEAD;
        return;
    }
    
    //Comer una manzana
    if (*head_ptr == GREEN){
        tail_index++;
        snake_parts[tail_index] = snake_parts[tail_index - 1] + 1;
        int * tail = snake_parts[tail_index];
        *tail = RED;
        create_apple();
    }
    
    for (int i = 1; i <= tail_index; i++){
        if (i == tail_index) {
            int *tail = snake_parts[i];
            *tail = BLACK; //La cola se hace negra para apreciar el movimiento
        }
        
        int *curr_part = snake_parts[i];
        snake_parts[i] = old_part;
        old_part = curr_part;
    }
    //Llenar la snake
    for (int i = 0; i <= tail_index; i++) {
        int *ptr = snake_parts[i];
        //Si es la cabeza que sea azul
        if (i == 0) {
            *ptr = BLUE;
        //Si es el resto del cuerpo que sea rojo 
        } else {
            *ptr = RED;
        }
    }
}

//Si se acaba el juego
boolean is_gameover(){
    return (game_state == SNAKE_DEAD || game_state == SNAKE_WIN || tail_index == (led_width * led_height));
}

//Actual Game
void main(){
    int offset = right_offset;
    game_state = SNAKE_ALIVE;
    
    while (1){
        init_snake();
        init_tablero();
        create_apple();
        
        while (!is_gameover()){
            int prev_offset = offset;
            for (int i = 0; i < 8000; i++) {} //delay
            if (*btn_up) offset = up_offset;
            if (*btn_down) offset = down_offset;
            if (*btn_left) offset = left_offset;
            if (*btn_right) offset = right_offset;
            
            //Revisar que no sean opuestos
            if (offset + prev_offset == 0) offset = prev_offset;
            move_snake(offset);
        }
        while (! *switch_0){}
        
        //Limpiar la pantalla
        for (int i = 0; i < led_size; i++){
            *(led_base + i) = BLACK;
        }
        
        //Reiniciar variables
        game_state = SNAKE_ALIVE;
        tail_index = INITIAL_SNAKE_SIZE - 1;
        rand_time = 50;
    }
}