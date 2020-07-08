#include "snakePi.h"

int flags = 0;



// Declaracion del objeto teclado
TipoTeclado teclado = {
		.columnas = {
				GPIO_KEYBOARD_COL_1,
				GPIO_KEYBOARD_COL_2,
				GPIO_KEYBOARD_COL_3,
				GPIO_KEYBOARD_COL_4,

		},
		.filas = {
				GPIO_KEYBOARD_ROW_1,
				GPIO_KEYBOARD_ROW_2,
				GPIO_KEYBOARD_ROW_3,
				GPIO_KEYBOARD_ROW_4

		},
		.rutinas_ISR = {
				teclado_fila_1_isr,
				teclado_fila_2_isr,
				teclado_fila_3_isr,
				teclado_fila_4_isr,

		},
		//Inicializo el resto de variables.
			.flags = 0,
			.debounceTime = { 0, 0, 0, 0 },
			.teclaPulsada = { -1, -1 },
			.columna_actual = COLUMNA_1,

};


// Declaracion del objeto display
TipoLedDisplay led_display = {
			.columnas = {
					GPIO_LED_DISPLAY_COL_1,
					GPIO_LED_DISPLAY_COL_2,
					GPIO_LED_DISPLAY_COL_3,
					0,0,0,0,0,

			},
			.filas = {
					GPIO_LED_DISPLAY_ROW_1,
					GPIO_LED_DISPLAY_ROW_2,
					GPIO_LED_DISPLAY_ROW_3,
					GPIO_LED_DISPLAY_ROW_4,
					GPIO_LED_DISPLAY_ROW_5,
					GPIO_LED_DISPLAY_ROW_6,
					GPIO_LED_DISPLAY_ROW_7,
			},
			.columna_actual = COLUMNA_DISPLAY_1,
			.flags = 0,

			};

//------------------------------------------------------
// FUNCIONES DE CONFIGURACION/INICIALIZACION
//------------------------------------------------------

// int ConfiguracionSistema (TipoSistema *p_sistema): procedimiento de configuracion
// e inicializacion del sistema.
// Realizará, entra otras, todas las operaciones necesarias para:
// configurar el uso de posibles librerías (e.g. Wiring Pi),
// configurar las interrupciones externas asociadas a los pines GPIO,
// configurar las interrupciones periódicas y sus correspondientes temporizadores,
// la inicializacion de los diferentes elementos de los que consta nuestro sistema,
// crear, si fuese necesario, los threads adicionales que pueda requerir el sistema
// como el thread de exploración del teclado del PC
int ConfiguraInicializaSistema(TipoSistema *p_sistema) {
	//int result = 0;

	piLock(STD_IO_BUFFER_KEY);
	wiringPiSetupGpio();
	piUnlock(STD_IO_BUFFER_KEY);

	// sets up the wiringPi library
	if (wiringPiSetupGpio() < 0) {
		printf("Unable to setup wiringPi\n");

		return -1;
	}


	//Inicializamos el display
	InicializaLedDisplay(&led_display);
	//Inicializamos el teclado
	InicializaTeclado(&teclado);


	printf("\nBienvenido a snakePi\n");
	printf("\nControl del juego: S para empezar\n");
	printf("D: Derecha, A: Izquierda, W: Arriba, X: Abajo, Z: Pausar, S: Reanudar\n");
	printf("Mejoras:\n");
	printf(" 1. - Te dice al acabar cuantas manzanas te has comido\n");
	printf(" 2. - Cada 3 manzanas que se come, la serpiente va mas rápido\n");
	printf(" 3. - Se puede pausar y reanudar el juego.\n");
	printf(" 4. - No hay bordes.");


	fflush(stdout);

	return 1;
}

//------------------------------------------------------
// FUNCIONES LIGADAS A THREADS ADICIONALES
//------------------------------------------------------



// wait until next_activation (absolute time)
void delay_until(unsigned int next) {
	unsigned int now = millis();
	if (next > now) {
		delay(next - now);
	}
}

/*
 * Extraido del ejemplo de las diapositivas de la version 2, disponibles
 * en Moodle.
 */

void timer_isr_snakePi(union sigval value) {
	piLock(SYSTEM_FLAGS_KEY);
	flags |= FLAG_TIMER_JUEGO;
	piUnlock(SYSTEM_FLAGS_KEY);
}

int main() {
	TipoSistema sistema;
	unsigned int next;

	fsm_trans_t snakePi[] = {
			{ WAIT_START, CompruebaBotonPulsado, WAIT_PUSH, InicializaJuego },
			{ WAIT_PUSH, CompruebaTimeoutActualizacionJuego,WAIT_PUSH, ActualizarJuego },
			{ WAIT_PUSH, CompruebaMovimientoArriba, WAIT_PUSH, MueveSerpienteArriba },
			{ WAIT_PUSH, CompruebaMovimientoAbajo, WAIT_PUSH,MueveSerpienteAbajo },
			{ WAIT_PUSH,CompruebaMovimientoIzquierda, WAIT_PUSH,MueveSerpienteIzquierda },
			{ WAIT_PUSH, CompruebaMovimientoDerecha, WAIT_PUSH,MueveSerpienteDerecha },
			{ WAIT_PUSH, CompruebaPausaJuego, WAIT_PAUSE, PausaJuego},
			{ WAIT_PAUSE, CompruebaBotonPulsado,WAIT_PUSH, ReanudarJuego },
			{ WAIT_PUSH, CompruebaFinalJuego,WAIT_END, FinalJuego },
			{ WAIT_END, CompruebaBotonPulsado,WAIT_START, ReseteaJuego },
			{ -1, NULL, -1, NULL },

	};

	// Configuracion e inicializacion del sistema
	ConfiguraInicializaSistema(&sistema);

	sistema.snakePi.p_pantalla = &(led_display.pantalla);

	//Temporizador del snake.
	sistema.snakePi.tmr_serpiente = tmr_new(timer_isr_snakePi);

	fsm_t *snakePi_fsm = fsm_new(WAIT_START, snakePi, &(sistema.snakePi));

	fsm_t *exc_col_fsm = fsm_new(TECLADO_ESPERA_COLUMNA,fsm_trans_excitacion_columnas, &(teclado));
	fsm_t *exc_tecla_fsm = fsm_new(TECLADO_ESPERA_TECLA,fsm_trans_deteccion_pulsaciones, &(teclado));
	fsm_t *display_fsm = fsm_new(DISPLAY_ESPERA_COLUMNA, fsm_trans_excitacion_display, &(led_display));

	next = millis();
	while (1) {
		fsm_fire(snakePi_fsm);
		fsm_fire(exc_col_fsm);
		fsm_fire(exc_tecla_fsm);
		fsm_fire(display_fsm);



		next += CLK_MS;
		delay_until(next);
	}

	fsm_destroy(snakePi_fsm);
	fsm_destroy(exc_col_fsm);
	fsm_destroy(exc_tecla_fsm);
	fsm_destroy(display_fsm);


	//Destruimos el tmr cuando se vuelva a empezar.
	tmr_destroy((tmr_t*)(snakePi_fsm->user_data));
	return 0;
}
