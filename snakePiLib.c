#include "snakePiLib.h"

//------------------------------------------------------
// PROCEDIMIENTOS DE INICIALIZACION DE LOS OBJETOS ESPECIFICOS
//------------------------------------------------------

//Variable nueva donde se almacena el numero de manzanas.
int numero_manzanas = 0;


void InicializaManzana(tipo_manzana *p_manzana) {
	// Aleatorizamos la posicion inicial de la manzana
	p_manzana->x = rand() % NUM_COLUMNAS_DISPLAY;
	p_manzana->y = rand() % NUM_FILAS_DISPLAY;
}

void InicializaSerpiente(tipo_serpiente *p_serpiente) {
	// Nos aseguramos de que la serpiente comienza sin p_cola
	LiberaMemoriaCola(p_serpiente);

	// Inicializamos la posicion inicial de la cabeza al comienzo del juego
	p_serpiente->p_cola->x = 3;
	p_serpiente->p_cola->y = 3;
	p_serpiente->direccion = NONE;
}

void InicializaSnakePi(tipo_snakePi *p_snakePi) {
	// Modelamos la serpiente como una p_cola de segmentos
	// Inicialmente la serpiente consta de un unico segmento: la cabeza
	// Actualizamos la p_cola para que incluya a la cabeza
	p_snakePi->serpiente.p_cola = &(p_snakePi->serpiente.cabeza);
	p_snakePi->serpiente.p_cola->p_next = NULL;

	ResetSnakePi(p_snakePi);


	ActualizaPantallaSnakePi(p_snakePi);

}

void ResetSnakePi(tipo_snakePi *p_snakePi) {
	numero_manzanas = 0;
	InicializaSerpiente(&(p_snakePi->serpiente));
	InicializaManzana(&(p_snakePi->manzana));
}

//------------------------------------------------------
// PROCEDIMIENTOS PARA LA GESTION DEL JUEGO
//------------------------------------------------------

void ActualizaColaSerpiente(tipo_serpiente *p_serpiente) {
	tipo_segmento *seg_i;

	// Recorremos los diferentes segmentos de que consta la serpiente
	// desde el comienzo de la cola hacia el final haciendo que cada segmento pase
	// a ocupar la posicion del que le precedia en la cola
	for (seg_i = p_serpiente->p_cola; seg_i->p_next; seg_i = seg_i->p_next) {
		seg_i->x = seg_i->p_next->x;
		seg_i->y = seg_i->p_next->y;
	}
}

int ActualizaLongitudSerpiente(tipo_serpiente *p_serpiente) {
	tipo_segmento *nueva_cola;

	nueva_cola = malloc(sizeof(tipo_segmento));

	if (!nueva_cola) {
		printf("[ERROR!!!][PROBLEMAS DE MEMORIA!!!]\n");
		return 0;
	}

	nueva_cola->x = p_serpiente->p_cola->x;
	nueva_cola->y = p_serpiente->p_cola->y;
	nueva_cola->p_next = p_serpiente->p_cola;

	p_serpiente->p_cola = nueva_cola;

	return 1;
}

int ActualizaSnakePi(tipo_snakePi *p_snakePi) {
	//tipo_segmento *seg_i;


	ActualizaColaSerpiente(&(p_snakePi->serpiente));

	if (CompruebaColision(&(p_snakePi->serpiente), &(p_snakePi->manzana), 1)) {
		// Colision con manzana, nos comemos la manzana y la serpiente crece
		if (!ActualizaLongitudSerpiente(&(p_snakePi->serpiente)))
			return 0;

		// Añadimos una nueva manzana asegurandonos de que
		// no aparezca en una posicion ya ocupada por la serpiente
		while (CompruebaColision(&(p_snakePi->serpiente), &(p_snakePi->manzana),
				1)) {
			InicializaManzana(&(p_snakePi->manzana));
		}
	}

	switch (p_snakePi->serpiente.direccion) {
	case ARRIBA:
		p_snakePi->serpiente.cabeza.y--;
		break;
	case DERECHA:
		p_snakePi->serpiente.cabeza.x++;
		break;
	case ABAJO:
		p_snakePi->serpiente.cabeza.y++;
		break;
	case IZQUIERDA:
		p_snakePi->serpiente.cabeza.x--;
		break;
	case NONE:
		break;
	}

	return 1;
}

void CambiarDireccionSerpiente(tipo_serpiente *serpiente,
		enum t_direccion direccion) {
	switch (direccion) {
	case ARRIBA:
		// No puedo cambiar de sentido! Me como!
		if (serpiente->direccion != ABAJO)
			serpiente->direccion = ARRIBA;
		break;
	case DERECHA:
		// No puedo cambiar de sentido! Me como!
		if (serpiente->direccion != IZQUIERDA)
			serpiente->direccion = DERECHA;
		break;
	case ABAJO:
		// No puedo cambiar de sentido! Me como!
		if (serpiente->direccion != ARRIBA)
			serpiente->direccion = ABAJO;
		break;
	case IZQUIERDA:
		// No puedo cambiar de sentido! Me como!
		if (serpiente->direccion != DERECHA)
			serpiente->direccion = IZQUIERDA;
		break;
	default:
		printf("[ERROR!!!!][direccion NO VALIDA!!!!][%d]", direccion);
		break;
	}
}


int CompruebaColision(tipo_serpiente *serpiente, tipo_manzana *manzana, int comprueba_manzana) {
	tipo_segmento *seg_i;

	if (comprueba_manzana) {
		// Para todos los elementos de la p_cola...
		for (seg_i = serpiente->p_cola; seg_i; seg_i=seg_i->p_next) {
			// ...compruebo si alguno ha "colisionado" con la manzana
			if (seg_i->x == manzana->x && seg_i->y == manzana->y){
				numero_manzanas = numero_manzanas +1;
				return 1;
			}
		}
	}
	else {
		// Compruebo si la cabeza de la serpiente colisiona con cualquier otro segmento de la cola...
		for(seg_i = serpiente->p_cola; seg_i->p_next; seg_i=seg_i->p_next) {
			if (serpiente->cabeza.x == seg_i->x && serpiente->cabeza.y == seg_i->y)
				return 1;
		}



		// Quitamos los bordes y asignamos el valor a recoger.
		if(serpiente->cabeza.x < 0){
			serpiente->cabeza.x = (NUM_COLUMNAS_DISPLAY - 1);
		}else if(serpiente->cabeza.x >= NUM_COLUMNAS_DISPLAY){
			serpiente->cabeza.x = 0;
		}else if(serpiente->cabeza.y < 0){
			serpiente->cabeza.y = (NUM_FILAS_DISPLAY - 1);
		}else if(serpiente->cabeza.y >= NUM_FILAS_DISPLAY){
			serpiente->cabeza.y = 0;
		}




	}
	return 0;
}

// serpiente.p_cola->seg_1->seg_2->seg_3->...->seg_N->NULL

void LiberaMemoriaCola(tipo_serpiente *p_serpiente) {
	tipo_segmento *seg_i;
	tipo_segmento *next_tail;

	seg_i = p_serpiente->p_cola;

	while (seg_i->p_next) {
		next_tail = seg_i->p_next;
		free(seg_i);
		seg_i = next_tail;
	}

	p_serpiente->p_cola = seg_i;
	p_serpiente->p_cola->p_next = NULL;
}

//------------------------------------------------------
// PROCEDIMIENTOS PARA LA VISUALIZACION DEL JUEGO
//------------------------------------------------------

void PintaManzana(tipo_snakePi *p_snakePi) {
	p_snakePi->p_pantalla->matriz[p_snakePi->manzana.x][p_snakePi->manzana.y] =
			1;
}

void PintaSerpiente(tipo_snakePi *p_snakePi) {
	tipo_segmento *seg_i;

	for (seg_i = p_snakePi->serpiente.p_cola; seg_i->p_next;
			seg_i = seg_i->p_next) {
		p_snakePi->p_pantalla->matriz[seg_i->x][seg_i->y] = 1;
	}

	p_snakePi->p_pantalla->matriz[seg_i->x][seg_i->y] = 1;
}

void ActualizaPantallaSnakePi(tipo_snakePi *p_snakePi) {

	// Borro toda la pantalla
	ReseteaPantallaSnakePi((tipo_pantalla*) (p_snakePi->p_pantalla));

	PintaManzana(p_snakePi);
	PintaSerpiente(p_snakePi);
}



void ReseteaPantallaSnakePi(tipo_pantalla *p_pantalla) {

	int i;
	int j;

	for (i = 0; i < NUM_COLUMNAS_DISPLAY; i++) {
		for (j = 0; j < NUM_FILAS_DISPLAY; j++) {
			p_pantalla->matriz[i][j] = 0;
		}
	}
}

//------------------------------------------------------
// FUNCIONES DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

int CompruebaBotonPulsado(fsm_t *this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags & FLAG_BOTON);
	piUnlock(SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaMovimientoArriba(fsm_t *this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags & FLAG_MOV_ARRIBA);
	piUnlock(SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaMovimientoAbajo(fsm_t *this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags & FLAG_MOV_ABAJO);
	piUnlock(SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaMovimientoIzquierda(fsm_t *this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags & FLAG_MOV_IZQUIERDA);
	piUnlock(SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaMovimientoDerecha(fsm_t *this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags & FLAG_MOV_DERECHA);
	piUnlock(SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaTimeoutActualizacionJuego(fsm_t *this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags & FLAG_TIMER_JUEGO);
	piUnlock(SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaFinalJuego(fsm_t *this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags & FLAG_FIN_JUEGO);
	piUnlock(SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaPausaJuego(fsm_t *this){
	int result = 0;
	piLock(SYSTEM_FLAGS_KEY);
	result = flags & FLAG_PAUSA_JUEGO;
	piUnlock(SYSTEM_FLAGS_KEY);

	return result;
}



//------------------------------------------------------
// FUNCIONES DE ACCION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

void InicializaJuego(fsm_t *this) {
	tipo_snakePi *p_snakePi;
	p_snakePi = (tipo_snakePi*) (this->user_data);



	piLock(SYSTEM_FLAGS_KEY);
	flags &= (~FLAG_MOV_IZQUIERDA);
	flags &= (~FLAG_MOV_DERECHA);
	flags &= (~FLAG_MOV_ARRIBA);
	flags &= (~FLAG_MOV_ABAJO);
	flags &= (~FLAG_BOTON);
	piUnlock(SYSTEM_FLAGS_KEY);



	piLock(MATRIX_KEY);
	//Ejecutamos el juego.
	InicializaSnakePi(p_snakePi);
	piUnlock(MATRIX_KEY);


	//Para ejecutar la simulacion del display
	pseudoWiringPiEnableDisplay(1);


}



void MueveSerpienteIzquierda(fsm_t *this) {
	tipo_snakePi *p_snakePi;
	p_snakePi = (tipo_snakePi*) (this->user_data);

	piLock(SYSTEM_FLAGS_KEY);
	flags &= (~FLAG_MOV_IZQUIERDA);
	piUnlock(SYSTEM_FLAGS_KEY);

	CambiarDireccionSerpiente(&(p_snakePi->serpiente), IZQUIERDA);
	ActualizaSnakePi(p_snakePi);

	// Creo que es a 1 y no a 0 segun entiendo el metodo CompruebaColision()
	if (CompruebaColision(&(p_snakePi->serpiente), &(p_snakePi->manzana), 0)) {
		piLock(SYSTEM_FLAGS_KEY);
		flags |= FLAG_FIN_JUEGO;
		piUnlock(SYSTEM_FLAGS_KEY);
	}

	else {

		piLock(MATRIX_KEY);
		ActualizaPantallaSnakePi(p_snakePi);
		piUnlock(MATRIX_KEY);

		if((numero_manzanas/2) < 3){
					tmr_startms((p_snakePi->tmr_serpiente), TIMEOUT);

				}else if((numero_manzanas/2) >= 3 && (numero_manzanas/2) < 6){
						tmr_startms((p_snakePi->tmr_serpiente), TIMEOUT_RAPIDO);
						}else if((numero_manzanas/2) >= 6){
							tmr_startms((p_snakePi->tmr_serpiente), TIMEOUT_RAPIDO2);
						}
					}
				}

void MueveSerpienteDerecha(fsm_t *this) {
	tipo_snakePi *p_snakePi;
	p_snakePi = (tipo_snakePi*) (this->user_data);

	piLock(SYSTEM_FLAGS_KEY);
	flags &= (~FLAG_MOV_DERECHA);
	piUnlock(SYSTEM_FLAGS_KEY);

	CambiarDireccionSerpiente(&(p_snakePi->serpiente), DERECHA);
	ActualizaSnakePi(p_snakePi);

	if (CompruebaColision(&(p_snakePi->serpiente), &(p_snakePi->manzana), 0)) {
		piLock(SYSTEM_FLAGS_KEY);
		flags |= FLAG_FIN_JUEGO;
		piUnlock(SYSTEM_FLAGS_KEY);
	}

	else {

		piLock(MATRIX_KEY);
		ActualizaPantallaSnakePi(p_snakePi);
		piUnlock(MATRIX_KEY);



		if((numero_manzanas/2) < 3){
					tmr_startms((p_snakePi->tmr_serpiente), TIMEOUT);

				}else if((numero_manzanas/2) >= 3 && (numero_manzanas/2) < 6){
						tmr_startms((p_snakePi->tmr_serpiente), TIMEOUT_RAPIDO);
						}else if((numero_manzanas/2) >= 6){
							tmr_startms((p_snakePi->tmr_serpiente), TIMEOUT_RAPIDO2);
						}
					}
				}
void MueveSerpienteArriba(fsm_t *this) {
	tipo_snakePi *p_snakePi;
	p_snakePi = (tipo_snakePi*) (this->user_data);

	piLock(SYSTEM_FLAGS_KEY);
	flags &= (~FLAG_MOV_ARRIBA);
	piUnlock(SYSTEM_FLAGS_KEY);

	CambiarDireccionSerpiente(&(p_snakePi->serpiente), ARRIBA);
	ActualizaSnakePi(p_snakePi);


	if (CompruebaColision(&(p_snakePi->serpiente), &(p_snakePi->manzana), 0)) {
		piLock(SYSTEM_FLAGS_KEY);
		flags |= FLAG_FIN_JUEGO;
		piUnlock(SYSTEM_FLAGS_KEY);
	}

	else {

		piLock(MATRIX_KEY);
		ActualizaPantallaSnakePi(p_snakePi);
		piUnlock(MATRIX_KEY);


		if((numero_manzanas/2) < 3){
					tmr_startms((p_snakePi->tmr_serpiente), TIMEOUT);

				}else if((numero_manzanas/2) >= 3 && (numero_manzanas/2) < 6){
						tmr_startms((p_snakePi->tmr_serpiente), TIMEOUT_RAPIDO);
						}else if((numero_manzanas/2) >= 6){
							tmr_startms((p_snakePi->tmr_serpiente), TIMEOUT_RAPIDO2);
						}
					}
				}

void MueveSerpienteAbajo(fsm_t *this) {
	tipo_snakePi *p_snakePi;
	p_snakePi = (tipo_snakePi*) (this->user_data);

	piLock(SYSTEM_FLAGS_KEY);
	flags &= (~FLAG_MOV_ABAJO);
	piUnlock(SYSTEM_FLAGS_KEY);

	CambiarDireccionSerpiente(&(p_snakePi->serpiente), ABAJO);
	ActualizaSnakePi(p_snakePi);


	if (CompruebaColision(&(p_snakePi->serpiente), &(p_snakePi->manzana), 0)) {
		piLock(SYSTEM_FLAGS_KEY);
		flags |= FLAG_FIN_JUEGO;
		piUnlock(SYSTEM_FLAGS_KEY);
	}

	else {

		piLock(MATRIX_KEY);
		ActualizaPantallaSnakePi(p_snakePi);
		piUnlock(MATRIX_KEY);


		if((numero_manzanas/2) < 3){
					tmr_startms((p_snakePi->tmr_serpiente), TIMEOUT);

				}else if((numero_manzanas/2) >= 3 && (numero_manzanas/2) < 6){
						tmr_startms((p_snakePi->tmr_serpiente), TIMEOUT_RAPIDO);
						}else if((numero_manzanas/2) >= 6){
							tmr_startms((p_snakePi->tmr_serpiente), TIMEOUT_RAPIDO2);
						}
					}
				}

void ActualizarJuego(fsm_t *this) {
	tipo_snakePi *p_snakePi;
	p_snakePi = (tipo_snakePi*) (this->user_data);

	piLock(SYSTEM_FLAGS_KEY);
	flags &= (~FLAG_TIMER_JUEGO);
	// Con esto apago el flag_boton y hago que si se da a la S no se reinicie.
	flags &= (~FLAG_BOTON);
	//flags &= (~FLAG_PAUSA_JUEGO);
	piUnlock(SYSTEM_FLAGS_KEY);



	piLock(MATRIX_KEY);
	ActualizaSnakePi(p_snakePi);
	piUnlock(MATRIX_KEY);


	if (CompruebaColision(&(p_snakePi->serpiente), &(p_snakePi->manzana), 0)) {
		piLock(SYSTEM_FLAGS_KEY);
		flags |= FLAG_FIN_JUEGO;
		piUnlock(SYSTEM_FLAGS_KEY);

	}

	else {


		piLock(MATRIX_KEY);
		ActualizaPantallaSnakePi(p_snakePi);

		piUnlock(MATRIX_KEY);


		if((numero_manzanas/2) < 3 ){
			tmr_startms((p_snakePi->tmr_serpiente), TIMEOUT);

		}else if((numero_manzanas/2) >= 3 && (numero_manzanas/2) < 6){
				tmr_startms((p_snakePi->tmr_serpiente), TIMEOUT_RAPIDO);
				}else if((numero_manzanas/2) >= 6){
					tmr_startms((p_snakePi->tmr_serpiente), TIMEOUT_RAPIDO2);
				}
			}
		}




void FinalJuego(fsm_t *this) {

	pseudoWiringPiEnableDisplay(0);

	tipo_snakePi *p_snakePi;
	p_snakePi = (tipo_snakePi*) (this->user_data);

	piLock(SYSTEM_FLAGS_KEY);
	flags &= (~FLAG_FIN_JUEGO);
	piUnlock(SYSTEM_FLAGS_KEY);


	piLock(MATRIX_KEY);

	ActualizaSnakePi(p_snakePi);

	piUnlock(MATRIX_KEY);


	printf("\nGAME OVER.\n");
	printf("Te has comido %d manzanas. \n", numero_manzanas/2);
	printf("Vuelve a pulsar la S para jugar");
	fflush(stdout);




}

void PausaJuego(fsm_t *this){
	//tipo_snakePi *p_snakePi;
	//p_snakePi = (tipo_snakePi*) (this->user_data);

	piLock(SYSTEM_FLAGS_KEY);

	flags &= ~FLAG_PAUSA_JUEGO;
	flags &= ~FLAG_BOTON;
	flags &= ~FLAG_TIMER_JUEGO;


	piUnlock(SYSTEM_FLAGS_KEY);





}

void ReanudarJuego(fsm_t *this){
	tipo_snakePi *p_snakePi;
	p_snakePi = (tipo_snakePi*) (this->user_data);

	piLock(SYSTEM_FLAGS_KEY);
	flags &= ~FLAG_BOTON;
	flags &= (~FLAG_MOV_IZQUIERDA);
	flags &= (~FLAG_MOV_DERECHA);
	flags &= (~FLAG_MOV_ARRIBA);
	flags &= (~FLAG_MOV_ABAJO);
	flags &= (~FLAG_BOTON);
	piUnlock(SYSTEM_FLAGS_KEY);

	//tmr_startms(p_snakePi->tmr_serpiente, TIMEOUT);


}



void ReseteaJuego(fsm_t *this) {

	piLock(SYSTEM_FLAGS_KEY);
	flags &= ~FLAG_TIMER_JUEGO;
	flags &= ~FLAG_FIN_JUEGO;
	flags &= ~FLAG_BOTON;
	piUnlock(SYSTEM_FLAGS_KEY);


	printf("\n");
	printf("\nPulse de nuevo S para empezar\n");
	printf("D: Derecha, A: Izquierda, W: Arriba, X: Abajo, Z: Pausar, S: Reanudar\n");
	printf("Mejoras:\n");
	printf(" 1. - Te dice al acabar cuantas manzanas te has comido\n");
	printf(" 2. - Cada 3 manzanas que se come, la serpiente va mas rápido\n");
	printf(" 3. - Se puede pausar y reanudar el juego.\n");
	printf(" 4. - No hay bordes.");


	fflush(stdout);


}

