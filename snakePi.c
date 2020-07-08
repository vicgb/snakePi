
#include "snakePi.h"

int flags = 0;

// Declaracion del objeto teclado
TipoTeclado teclado = {
	.columnas = {
		// A completar por el alumno...
		// ...
	},
	.filas = {
		// A completar por el alumno...
		// ...
	},
	.rutinas_ISR = {
		// A completar por el alumno...
		// ...
	},

	// A completar por el alumno...
	// ...
};

// Declaracion del objeto display
TipoLedDisplay led_display = {
	.columnas = {
		// A completar por el alumno...
		// ...
	},
	.filas = {
		// A completar por el alumno...
		// ...
	},
	// A completar por el alumno...
	// ...
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
int ConfiguraInicializaSistema (TipoSistema *p_sistema) {
	int result = 0;

	piLock(STD_IO_BUFFER_KEY);
	wiringPiSetupGpio();
	piUnlock(STD_IO_BUFFER_KEY);

	// sets up the wiringPi library
	if (wiringPiSetupGpio () < 0) {
		printf ("Unable to setup wiringPi\n");
		return -1;
	}

	// Lanzamos thread para exploracion del teclado convencional del PC
	result = piThreadCreate (thread_explora_teclado_PC);

	if (result != 0) {
		printf ("Thread didn't start!!!\n");
		return -1;
	}

	printf("Sistema listo...\n");
	printf("Presione una tecla para comenzar (i.e. la tecla s)...\n");
	fflush(stdout);

	return result;
}

//------------------------------------------------------
// FUNCIONES LIGADAS A THREADS ADICIONALES
//------------------------------------------------------

PI_THREAD (thread_explora_teclado_PC) {
	int teclaPulsada;

	while(1) {
		delay(10); // Wiring Pi function: pauses program execution for at least 10 ms

		piLock (STD_IO_BUFFER_KEY);

		if(kbhit()) {
			teclaPulsada = kbread();

			switch(teclaPulsada) {

				case 's':

					printf("Comienza el juego\n");
					fflush(stdout);
					piLock(SYSTEM_FLAGS_KEY);
					flags |= FLAG_BOTON;
					piUnlock(SYSTEM_FLAGS_KEY);


					break;
				case 'w':

					printf("Moviendo hacia arriba\n");
					fflush(stdout);
					piLock(SYSTEM_FLAGS_KEY);
					flags |= FLAG_MOV_ARRIBA;
					piUnlock(SYSTEM_FLAGS_KEY);

					break;
				case 'x':
					printf("Moviendo hacia abajo\n");
					fflush(stdout);
					piLock(SYSTEM_FLAGS_KEY);
					flags |= FLAG_MOV_ABAJO;
					piUnlock(SYSTEM_FLAGS_KEY);

					break;
				case 'd':
					printf("Moviendo hacia la derecha\n");
					fflush(stdout);
					piLock(SYSTEM_FLAGS_KEY);
					flags |= FLAG_MOV_DERECHA;
					piUnlock(SYSTEM_FLAGS_KEY);

					break;
				case 'a':
					printf("Moviendo hacia la izquierda\n");
					fflush(stdout);
					piLock(SYSTEM_FLAGS_KEY);
					flags |= FLAG_MOV_IZQUIERDA;
					piUnlock(SYSTEM_FLAGS_KEY);

					break;

				case 'q':
					printf("Saliendo del juego.");
					exit(0);
					break;

				default:

					break;
			}
		}

		piUnlock (STD_IO_BUFFER_KEY);
	}
}

// wait until next_activation (absolute time)
void delay_until (unsigned int next) {
	unsigned int now = millis();
	if (next > now) {
		delay (next - now);
	}
}

int main () {
	TipoSistema sistema;
	unsigned int next;

	fsm_trans_t snakePi[] = {
		{ WAIT_START, CompruebaBotonPulsado, WAIT_PUSH, InicializaJuego },
		{ WAIT_PUSH, CompruebaTimeoutActualizacionJuego, WAIT_PUSH, ActualizarJuego },
		{ WAIT_PUSH, CompruebaMovimientoArriba, WAIT_PUSH, MueveSerpienteArriba },
		{ WAIT_PUSH, CompruebaMovimientoAbajo, WAIT_PUSH, MueveSerpienteAbajo },
		{ WAIT_PUSH, CompruebaMovimientoIzquierda, WAIT_PUSH, MueveSerpienteIzquierda },
		{ WAIT_PUSH, CompruebaMovimientoDerecha, WAIT_PUSH, MueveSerpienteDerecha },
		{ WAIT_PUSH, CompruebaFinalJuego, WAIT_END, FinalJuego },
		{ WAIT_END, CompruebaBotonPulsado, WAIT_START, ReseteaJuego },
		{-1, NULL, -1, NULL },
	};

	// Configuracion e inicializacion del sistema
	ConfiguraInicializaSistema (&sistema);

	fsm_t* snakePi_fsm = fsm_new (WAIT_START, snakePi, &(sistema.snakePi));
	sistema.snakePi.p_pantalla = &pantalla_inicial;


	next = millis();
	while (1) {
		fsm_fire (snakePi_fsm);

		// A completar por el alumno...
		// ...

		next += CLK_MS;
		delay_until (next);
	}

	fsm_destroy (snakePi_fsm);

	return 0;
}
