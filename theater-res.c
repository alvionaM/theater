#include "p3200098-p3200150-res.h"

unsigned int init_seed;
int iban = 0;
int tickets_failed = 0;
int credit_failed_zA = 0;
int credit_failed_zB = 0;
int txn_completed = 0;

long tel_standby = 0;
long cash_standby = 0;
long total_serving_time = 0;

int avail_lines = N_tel;
pthread_mutex_t line_mutex;
pthread_cond_t line_cond;

int avail_cash  = N_cash;
pthread_mutex_t cash_mutex;
pthread_cond_t cash_cond;


// Seats table
int seats[N_totalSeats];
int avail_seats_pRow[N_zoneA + N_zoneB];
 

// Table mutexes
pthread_mutex_t zoneA_mutex, zoneB_mutex;

// Transaction mutex
pthread_mutex_t txn_mutex;
	
// Print mutex
pthread_mutex_t print_mutex;




int main(int argc, char** argv) {
	int N_cust;
	pthread_t* customers;
	int* cust_id;
	unsigned int seed;
	int i, rc;


	if (argc != 3) {
		printf("You need to enter N_cust and seed!\n");
		return -1;
	}

	N_cust = atoi(argv[1]);
	if (N_cust <= 0 ) {
		printf("N_cust must be positive integer!\n");
		return -1;
	}

	init_seed = atoi(argv[2]);
	seed = init_seed;
	
	// Seats table init
	for (i = 0; i < N_totalSeats; ++i) {
		seats[i] = 0;
	}
	// Avail_seats table init
	for (i = 0; i < N_zoneA + N_zoneB; ++i) {
		avail_seats_pRow[i] = N_seat;
	}

	// Mutex Init
	mutex_init(&line_mutex, NULL);
	mutex_init(&cash_mutex, NULL);
	mutex_init(&zoneA_mutex, NULL);
	mutex_init(&zoneB_mutex, NULL);
	mutex_init(&txn_mutex, NULL);
	mutex_init(&print_mutex, NULL);

	// Cond Init
	cond_init(&line_cond, NULL);
	cond_init(&cash_cond, NULL);

	int try = 0;
	do {
		customers = (pthread_t*) malloc(N_cust * sizeof(pthread_t));
		try++;
	}
	while (customers == NULL && try < 5);

	try = 0;
	do {
		cust_id = (int*) malloc(N_cust * sizeof(int));
		try++;
	}
	while (cust_id == NULL && try < 5);

	if (customers == NULL || cust_id == NULL) {
		printf("ERROR: memory could not be allocated\n");
		return -1;
	}


	// Threads creation
	for (i = 0; i < N_cust; ++i) {
		cust_id[i] = i+1;

		rc = pthread_create(&customers[i], NULL, reservation, &cust_id[i]);
			if (rc != 0) {
				printf("ERROR: return code from pthread_create() is %d\n", rc);
				return -1;
			}

		int res_time = t_reslow + rand_r(&seed) % (t_reshigh - t_reslow + 1);
		if (i != N_cust-1) {
			sleep(res_time);
		}
	}


	for (i = 0; i < N_cust; ++i) {
		
		rc = pthread_join(customers[i], NULL);
			if (rc != 0) {
				printf("ERROR: return code from pthread_join() is %d\n", rc);
				return -1;
			}

	}

	// Ektypwsi planou
	printf("\n");
	for (i = 0; i < N_totalSeats; ++i) {
		if (seats[i] != 0)
			printf("Zone %c / Line %d / Seat %d / Customer %d \n", (i < N_zoneA * N_seat) ? 'A' : 'B', i / N_seat, i % N_seat, seats[i]);
	}

	// Sunolika esoda
	printf("\nBox office: %d.00 $\n", iban);
	printf("\n");

	// Pososta
	printf("Number of completed reservations: %.2f %%\n", (txn_completed * 100.f) / N_cust);
	printf("Number of canceled reservations due to restricted availability: %.2f %%\n", (tickets_failed * 100.f) / N_cust);
	printf("Number of canceled reservations due to credit card's failure: %.2f %%\n", ((credit_failed_zA+credit_failed_zB) * 100.f) / N_cust);
	printf("\n");

	// Mesos xronos anamonhs
	printf("Average standby time per customer: %.2f \n", (tel_standby + cash_standby) * 1.0f / N_cust);

	// Mesos xronos eksuphrethshs
	printf("Average serving time per customer: %.2f \n", total_serving_time * 1.0f / N_cust);
	printf("\n");

	// Pinakas thesewn
	printf(" \t");
	for (i = 0; i < N_seat; ++i)
		// Sthles
		printf("%d\t", i);
	for (i = 0; i < N_totalSeats; ++i) {
		if (i % 10 == 0) 
			// Grammh
			printf("\n%d\t", i/10);
		// Thesi
		printf("%d\t", seats[i]);
	}
	printf("\n");

	free(cust_id);
	free(customers);
	mutex_destroy(&line_mutex);
	mutex_destroy(&cash_mutex);
	mutex_destroy(&zoneA_mutex);
	mutex_destroy(&zoneB_mutex);	
	mutex_destroy(&txn_mutex);
	mutex_destroy(&print_mutex);
	cond_destroy(&line_cond);
	cond_destroy(&cash_cond);

	return 0;
}










void* reservation(void* id) {
	int cust_id = *(int*)id;
	unsigned int this_seed = init_seed + cust_id;
	int cust_cost;
	struct timespec tel_standby_start, tel_standby_stop, cash_standby_start, cash_standby_stop, end_call;
	
	zones zone = (rand_r(&this_seed) / (RAND_MAX*1.0f)) <= P_zoneA ? zoneA : zoneB;
	int tickets = N_seatlow + rand_r(&this_seed) % (N_seathigh - N_seatlow + 1);
	

	// Anamonh thlefwnhth
	mutex_lock(&line_mutex);

		// Ekkinhsh anamonhs thlefwnhth
		clock_gettime(CLOCK_REALTIME, &tel_standby_start);

		while ( avail_lines == 0 ) {
			cond_wait(&line_cond, &line_mutex);
		}

		// Pairnei thlefwnhth
		avail_lines--;

		// Telos anamonhs thelefwnhth
		clock_gettime(CLOCK_REALTIME, &tel_standby_stop);

	mutex_unlock(&line_mutex);


	bool flag = false; 
	int first_seat;

	phone_operator(&this_seed, cust_id, zone, tickets, &first_seat, &flag);
	

	// Telos klhshs (de vrethikan eisithria) 
	if (flag == false) clock_gettime(CLOCK_REALTIME, &end_call);

// AVAIL_ LINES ++
	mutex_lock(&line_mutex);

		// Apeleutherwnei line
		avail_lines++;
		// Aukshsh metrhth sunolikhs anamonhs thlefwnhth
		tel_standby += tel_standby_stop.tv_sec - tel_standby_start.tv_sec;


		// De vrethikan eisithria!
		if (flag == false) {
			tickets_failed++;
			
			// Aukshsh metrhth sunolikou xronou eksuphrethshs
			total_serving_time += end_call.tv_sec - tel_standby_start.tv_sec;
		}

		cond_signal(&line_cond);

	mutex_unlock(&line_mutex);

//

	if (flag == false) {
		//Termatizei h klhsh - den vrethikan eisithria
		mutex_lock(&print_mutex);

			printf("Customer: %d --> Reservation failed - Requested seats are unavailable!\n", cust_id);

		mutex_unlock(&print_mutex);

		pthread_exit(0);
	}

	cust_cost = ((zone == zoneA) ? C_zoneA : C_zoneB) * tickets;


//==================================================================================
//==================================================================================
//==================================================================================

	// Anamonh tamia
	mutex_lock(&cash_mutex);

		// Ekkinhsh anamonhs tamia
		clock_gettime(CLOCK_REALTIME, &cash_standby_start);

		while ( avail_cash == 0 ) {
			cond_wait(&cash_cond, &cash_mutex);
		}

		// Pairnei tamia
		avail_cash--;

		// Telos anamonhs tamia
		clock_gettime(CLOCK_REALTIME, &cash_standby_stop);

	mutex_unlock(&cash_mutex);


	cashier(&this_seed, cust_id, zone, tickets, first_seat, cust_cost);
	

	// Telos klhshs
	clock_gettime(CLOCK_REALTIME, &end_call);


// AVAIL_CASH ++
	mutex_lock(&cash_mutex);

		// Apeleutherwnei tamia
		avail_cash++;
		// Aukshsh metrhth sunolikhs anamonhs tamia
		cash_standby += cash_standby_stop.tv_sec - cash_standby_start.tv_sec;
		// Aukshsh metrhth sunolikou xronou eksuphrethshs
		total_serving_time += end_call.tv_sec - tel_standby_start.tv_sec;

		cond_signal(&cash_cond);

	mutex_unlock(&cash_mutex);

//
	pthread_exit(0);
}









void phone_operator(unsigned int* this_seed, int cust_id, zones zone, int tickets, int* first_seat, bool* flag) {
	int i;

	// SLEEP
	int seat_time = t_seatlow + rand_r(this_seed) % (t_seathigh - t_seatlow + 1);
	sleep(seat_time);


	if (zone == zoneA) {
		mutex_lock(&zoneA_mutex);
	}
	else  {
		mutex_lock(&zoneB_mutex);			
	}

		*first_seat = seats_searching(zone, tickets, flag);
	
		// Enhmerwsh planou thesewn 
		if (*flag == true) {
			for (i = *first_seat; i < *first_seat+tickets; ++i) {
				seats[i] = cust_id;
			}

			// Enhmerwsh monodiastatou
			avail_seats_pRow[*first_seat / N_seat] -= tickets;	
		}


	if (zone == zoneA) { 
		mutex_unlock(&zoneA_mutex);
	}
	else {
		mutex_unlock(&zoneB_mutex);
	}
	
}


void cashier(unsigned int* this_seed, int cust_id, zones zone, int tickets, int first_seat, int cust_cost) {
	char seats_str[10];
	int i;

	bool succ_txn = (rand_r(this_seed) / (RAND_MAX*1.0f)) <= P_cardsuccess ? true : false;

	// SLEEP
	int cash_time = t_cashlow + rand_r(this_seed) % (t_cashhigh - t_cashlow + 1);
	sleep(cash_time);


	if (succ_txn == true) {

		// Plhrwmh
		mutex_lock(&txn_mutex);

			// Enhmerwsh logariasmou
			iban = iban + cust_cost;
			// Enhmerwsh epityxwn sunallagwn
			txn_completed++;

		mutex_unlock(&txn_mutex);


		// Mhnuma ston pelath
		mutex_lock(&print_mutex);

			printf("Customer: %d --> Zone: %c - Line: %d - Seats: %s - Cost: %d \n", cust_id, (zone == zoneA) ? 'A' : 'B', first_seat / N_seat, seats_string(tickets, first_seat, seats_str), cust_cost);

		mutex_unlock(&print_mutex);

	}
	else {

		if (zone == zoneA) {
			mutex_lock(&zoneA_mutex);
		}
		else {
			mutex_lock(&zoneB_mutex);
		}
			

			// Enhmerwsh planou thesewn 
			for (i = first_seat; i < first_seat+tickets; ++i) {
				seats[i] = 0;
			}
			// Enhmerwsh monodiastatou
			avail_seats_pRow[first_seat / N_seat] += tickets;
			// Enhmerwsh apotyxhmenwn sunallagwn
			if (zone == zoneA)	
				credit_failed_zA++;
			else
				credit_failed_zB++;


		if (zone == zoneA) {
			mutex_unlock(&zoneA_mutex);
		}
		else { 
			mutex_unlock(&zoneB_mutex);
		}

		// Mhnuma ston pelath
		mutex_lock(&print_mutex);

			printf("Customer: %d --> Reservation failed - Card declined!\n", cust_id);

		mutex_unlock(&print_mutex);

	}
}




int seats_searching(zones zone, int tickets, bool* flag) {
	int i, j;
	int curr_seat, succ_seats_found = 0;
	int first_seat; 	//to return

	int low = (zone == zoneA) ? zoneA_low : zoneB_low;
	int upper = (zone == zoneA) ? zoneA_upper : zoneB_upper;

	for (i = low; i < upper; ++i) {
		if (avail_seats_pRow[i] >= tickets) {
			for (j = 0; j < N_seat; ++j) {
				curr_seat = i * N_seat + j;

				if (seats[curr_seat] != 0) {
					succ_seats_found = 0;
				}
				else {
					succ_seats_found++;

					if (succ_seats_found == tickets) {
						*flag = true;
						first_seat = curr_seat - tickets + 1;
						break;
					}
				}
			}
			succ_seats_found = 0;
		}
		if (*flag == true) break;
	}

	return first_seat;
}


char* seats_string(int tickets, int first_seat, char seats_string[]) {
	int f_seat = first_seat % N_seat;
	int i;

	for (i = 0; i/2 < tickets; i+=2) {
		seats_string[i] = f_seat+'0';
		seats_string[i+1] = ' ';
		f_seat++;
	}
	seats_string[i-1] = '\0';

	return seats_string;
}



void mutex_init(pthread_mutex_t* mt, const pthread_mutexattr_t* mt_attr) {

	int rc = pthread_mutex_init(mt, NULL);

		if (rc != 0) {
			printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
			exit(-1);
		}
}


void cond_init(pthread_cond_t* cnd, const pthread_condattr_t* cnd_attr) {

	int rc = pthread_cond_init(cnd, NULL);

		if (rc != 0) {
			printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
			exit(-1);
		}
}


void mutex_lock(pthread_mutex_t* mt) {

	int rc = pthread_mutex_lock(mt);

		if (rc != 0) {
			printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
			pthread_exit(&rc);
		}
}


void mutex_unlock(pthread_mutex_t* mt) {

	int rc = pthread_mutex_unlock(mt);
	
		if (rc != 0) {
			printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
			pthread_exit(&rc);
		}
}


void cond_wait(pthread_cond_t* cnd, pthread_mutex_t* mt) {

	int rc = pthread_cond_wait(cnd, mt);
			
		if (rc != 0) {
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			pthread_exit(&rc);
		}

}


void cond_signal(pthread_cond_t* cnd) {

	int rc = pthread_cond_signal(cnd);
			
		if (rc != 0) {
			printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
			pthread_exit(&rc);
		}
}


void mutex_destroy(pthread_mutex_t* mt) {
  	int rc = pthread_mutex_destroy(mt);
		
		if (rc != 0) {
			printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
			exit(-1);		
		}

}


void cond_destroy(pthread_cond_t* cnd) {
	
	int rc = pthread_cond_destroy(cnd);
	
		if (rc != 0) {
			printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
			exit(-1);		
		}
}
