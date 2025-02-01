#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "queue_utils.h"

/* --- Global Simulation Variables --- */

// Global simulation time in minutes (0 to 60)
volatile int current_minute = 0;

// Mutex and condition variable to control simulation time advancement
pthread_mutex_t time_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t time_cond = PTHREAD_COND_INITIALIZER;

// Mutex to protect the global seating chart and seat assignment
pthread_mutex_t seat_mutex = PTHREAD_MUTEX_INITIALIZER;

// Total seats available and seats sold count
#define ROWS 10
#define COLS 10
#define TOTAL_SEATS (ROWS * COLS)
#define TOTAL_SELLERS 10
int total_seats_sold = 0;

// The concert seating chart. Unassigned seats are marked with "----".
// Each assigned seat holds the customer id.
char seating[ROWS][COLS][8];

// Structure to record a seat position.
typedef struct
{
	int row;
	int col;
} SeatPosition;

// Ordering arrays for each seller type (100 positions each).
SeatPosition order_H[TOTAL_SEATS];
SeatPosition order_M[TOTAL_SEATS];
SeatPosition order_L[TOTAL_SEATS];

// Global indices for the next seat to assign for each seller type.
int index_H = 0, index_M = 0, index_L = 0;

/* --- Seller Data Structure --- */

typedef struct
{
	char type;				// 'H', 'M', or 'L'
	int id;					// For M and L sellers; H seller uses id 0.
	int num_customers;		// Number of customers to expect (command-line parameter)
	Queue *queue;			// Queue of waiting customers
	int next_arrival_index; // Index to track arrival times array
	int *arrival_times;		// Array (size=num_customers) of random arrival times (0-59)
	int customer_count;		// To generate customer IDs sequentially
	// Metrics:
	int served;				   // Number of customers served (ticket sold)
	int turned_away;		   // Number of customers turned away (sold out or simulation end)
	int total_response_time;   // Sum of response times (start service - arrival)
	int total_turnaround_time; // Sum of turnaround times (completion - arrival)
} Seller;

/* --- Function Prototypes --- */
void initialize_seating_chart();
void initialize_orderings();
void print_seating_chart(int time);
void *timer_thread(void *arg);
void *seller_thread(void *arg);
void generate_arrival_times(Seller *seller);

/* --- Utility: Print current seating chart --- */
void print_seating_chart(int time)
{
	printf("%d:%02d Concert Seating Chart:\n", time / 60, time % 60);
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			printf("%s ", seating[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

/* --- Initialize the seating chart with "----" (empty seat) --- */
void initialize_seating_chart()
{
	for (int i = 0; i < ROWS; i++)
		for (int j = 0; j < COLS; j++)
			strcpy(seating[i][j], "----");
}

/* --- Initialize the seat assignment order for each seller type --- */
void initialize_orderings()
{
	int pos = 0;
	// For high-priced seller (H): assign front-to-back (row 0 to 9, col 0 to 9)
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			order_H[pos].row = i;
			order_H[pos].col = j;
			pos++;
		}
	}
	pos = 0;
	// For low-priced sellers (L): assign from back-to-front (row 9 to 0)
	for (int i = ROWS - 1; i >= 0; i--)
	{
		for (int j = 0; j < COLS; j++)
		{
			order_L[pos].row = i;
			order_L[pos].col = j;
			pos++;
		}
	}
	pos = 0;
	// For medium-priced sellers (M): custom order:
	// Order rows: 5,6,4,7,3,8,2,9,1,10 in 1-indexed terms.
	// In 0-indexed, that becomes: 4,5,3,6,2,7,1,8,0,9.
	int m_order[ROWS] = {4, 5, 3, 6, 2, 7, 1, 8, 0, 9};
	for (int r = 0; r < ROWS; r++)
	{
		int row = m_order[r];
		for (int j = 0; j < COLS; j++)
		{
			order_M[pos].row = row;
			order_M[pos].col = j;
			pos++;
		}
	}
}

/* --- Timer thread: advances simulation time once every 0.1 second --- */
void *timer_thread(void *arg)
{
	(void)arg; // unused
	while (1)
	{
		usleep(100000); // 0.1 sec per simulated minute (simulation takes about 6 sec total)
		pthread_mutex_lock(&time_mutex);
		current_minute++;
		pthread_cond_broadcast(&time_cond);
		pthread_mutex_unlock(&time_mutex);
		if (current_minute > 60)
		{
			printf("\nEnding timer thread\n");
			break;
		}
	}
	return NULL;
}

/* --- Generate random arrival times (in minutes [0,59]) for a seller and sort them --- */
void generate_arrival_times(Seller *seller)
{
	seller->arrival_times = (int *)malloc(seller->num_customers * sizeof(int));
	if (!seller->arrival_times)
	{
		perror("Failed to allocate arrival times array");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < seller->num_customers; i++)
	{
		seller->arrival_times[i] = rand() % 60;
	}
	// Sort the arrival times (simple bubble sort for small N)
	for (int i = 0; i < seller->num_customers - 1; i++)
	{
		for (int j = i + 1; j < seller->num_customers; j++)
		{
			if (seller->arrival_times[i] > seller->arrival_times[j])
			{
				int temp = seller->arrival_times[i];
				seller->arrival_times[i] = seller->arrival_times[j];
				seller->arrival_times[j] = temp;
			}
		}
	}
}

void *seller_thread(void *arg)
{
	Seller *seller = (Seller *)arg;
	int busy = 0;			  // 0: not serving any customer; 1: currently serving
	int service_end_time = 0; // When the current service will complete
	Customer *current_customer = NULL;
	int last_tick = -1; // The last minute that we processed

	while (1)
	{
		// Wait for the next minute tick if one exists, or exit if simulation is over.
		pthread_mutex_lock(&time_mutex);
		while (current_minute <= last_tick && current_minute <= 60)
		{
			pthread_cond_wait(&time_cond, &time_mutex);
		}
		// When we wake up, if simulation time is over and no service is in progress, exit.
		if (current_minute > 60 && !busy)
		{
			pthread_mutex_unlock(&time_mutex);
			// Clear any remaining waiting customers.
			while (!isEmpty(seller->queue))
			{
				Customer *cust = dequeue(seller->queue);
				printf("%d:%02d Seller %c%d: Customer %s turned away (end of simulation)\n",
					   current_minute / 60, current_minute % 60,
					   seller->type, (seller->type == 'H' ? 0 : seller->id), cust->id);
				fflush(stdout);
				seller->turned_away++;
				free(cust);
			}
			break;
		}
		int local_time = current_minute;
		last_tick = local_time; // Update the last processed tick
		pthread_mutex_unlock(&time_mutex);

		// Process any customer arrivals scheduled for this minute.
		while (seller->next_arrival_index < seller->num_customers &&
			   seller->arrival_times[seller->next_arrival_index] == local_time)
		{
			Customer *cust = (Customer *)malloc(sizeof(Customer));
			if (!cust)
			{
				perror("Failed to allocate Customer");
				exit(EXIT_FAILURE);
			}
			cust->arrival_time = local_time;
			seller->customer_count++;
			// Create customer ID depending on seller type.
			if (seller->type == 'H')
			{
				snprintf(cust->id, sizeof(cust->id), "H%03d", seller->customer_count);
			}
			else if (seller->type == 'M')
			{
				snprintf(cust->id, sizeof(cust->id), "M%03d", seller->id * 100 + seller->customer_count);
			}
			else if (seller->type == 'L')
			{
				snprintf(cust->id, sizeof(cust->id), "L%03d", seller->id * 100 + seller->customer_count);
			}
			printf("%d:%02d Seller %c%d: Customer %s arrives\n",
				   local_time / 60, local_time % 60, seller->type, (seller->type == 'H' ? 0 : seller->id), cust->id);
			fflush(stdout);
			enqueue(seller->queue, cust);
			seller->next_arrival_index++;
		}

		// If currently busy, check whether the sale is finished.
		if (busy)
		{
			if (local_time >= service_end_time)
			{
				// Sale is complete.
				current_customer->service_end_time = service_end_time;
				int turnaround = service_end_time - current_customer->arrival_time;
				seller->total_turnaround_time += turnaround;
				printf("%d:%02d Seller %c%d: Customer %s completed purchase and leaves\n",
					   local_time / 60, local_time % 60, seller->type, (seller->type == 'H' ? 0 : seller->id), current_customer->id);
				fflush(stdout);
				free(current_customer);
				current_customer = NULL;
				busy = 0;
			}
		}

		// If not busy and there is a waiting customer, try to serve them.
		if (!busy && !isEmpty(seller->queue))
		{
			Customer *cust = dequeue(seller->queue);
			// Try to assign a seat.
			pthread_mutex_lock(&seat_mutex);
			int assigned = 0;
			int row = -1, col = -1;
			if (total_seats_sold < TOTAL_SEATS)
			{
				if (seller->type == 'H')
				{
					while (index_H < TOTAL_SEATS)
					{
						row = order_H[index_H].row;
						col = order_H[index_H].col;
						if (strcmp(seating[row][col], "----") == 0)
						{
							strcpy(seating[row][col], cust->id);
							index_H++;
							assigned = 1;
							break;
						}
						index_H++;
					}
				}
				else if (seller->type == 'M')
				{
					while (index_M < TOTAL_SEATS)
					{
						row = order_M[index_M].row;
						col = order_M[index_M].col;
						if (strcmp(seating[row][col], "----") == 0)
						{
							strcpy(seating[row][col], cust->id);
							index_M++;
							assigned = 1;
							break;
						}
						index_M++;
					}
				}
				else if (seller->type == 'L')
				{
					while (index_L < TOTAL_SEATS)
					{
						row = order_L[index_L].row;
						col = order_L[index_L].col;
						if (strcmp(seating[row][col], "----") == 0)
						{
							strcpy(seating[row][col], cust->id);
							index_L++;
							assigned = 1;
							break;
						}
						index_L++;
					}
				}
				if (assigned)
				{
					total_seats_sold++;
				}
			}
			pthread_mutex_unlock(&seat_mutex);

			if (!assigned)
			{
				// No seat available: turn customer away.
				printf("%d:%02d Seller %c%d: Customer %s turned away (sold out)\n",
					   local_time / 60, local_time % 60, seller->type, (seller->type == 'H' ? 0 : seller->id), cust->id);
				fflush(stdout);
				seller->turned_away++;
				free(cust);
			}
			else
			{
				// Seat assigned: begin service.
				cust->service_start_time = local_time;
				int response = local_time - cust->arrival_time;
				seller->total_response_time += response;
				// Determine service duration based on seller type.
				int service_duration = 0;
				if (seller->type == 'H')
					service_duration = (rand() % 2) + 1; // 1 or 2 minutes
				else if (seller->type == 'M')
					service_duration = (rand() % 3) + 2; // 2, 3, or 4 minutes
				else if (seller->type == 'L')
					service_duration = (rand() % 4) + 4; // 4, 5, 6, or 7 minutes

				cust->service_duration = service_duration;
				service_end_time = local_time + service_duration;
				printf("%d:%02d Seller %c%d: Serving customer %s, assigned seat at row %d, col %d (Service time: %d minute%s)\n",
					   local_time / 60, local_time % 60, seller->type, (seller->type == 'H' ? 0 : seller->id),
					   cust->id, row + 1, col + 1, service_duration, service_duration > 1 ? "s" : "");
				fflush(stdout);
				print_seating_chart(local_time);
				busy = 1;
				current_customer = cust;
				seller->served++;
			}
		}
	}
	return NULL;
}

/* --- Main Function --- */
int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <number_of_customers_per_seller>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	int N = atoi(argv[1]);
	if (N <= 0)
	{
		fprintf(stderr, "Number of customers must be a positive integer.\n");
		exit(EXIT_FAILURE);
	}

	srand(0); // Seed the random number generator

	initialize_seating_chart();
	initialize_orderings();

	// Total sellers: 1 H, 3 M, 6 L.
	Seller sellers[TOTAL_SELLERS];
	pthread_t seller_threads[TOTAL_SELLERS];
	int t = 0; // thread index

	// Initialize H seller (high-priced)
	sellers[t].type = 'H';
	sellers[t].id = 0;
	sellers[t].num_customers = N;
	sellers[t].queue = initQueue();
	sellers[t].next_arrival_index = 0;
	sellers[t].customer_count = 0;
	sellers[t].served = sellers[t].turned_away = 0;
	sellers[t].total_response_time = sellers[t].total_turnaround_time = 0;
	generate_arrival_times(&sellers[t]);
	pthread_create(&seller_threads[t], NULL, seller_thread, &sellers[t]);
	t++;

	// Initialize 3 medium-priced sellers (M)
	for (int i = 1; i <= 3; i++)
	{
		sellers[t].type = 'M';
		sellers[t].id = i; // For ID generation: M101, M102, etc.
		sellers[t].num_customers = N;
		sellers[t].queue = initQueue();
		sellers[t].next_arrival_index = 0;
		sellers[t].customer_count = 0;
		sellers[t].served = sellers[t].turned_away = 0;
		sellers[t].total_response_time = sellers[t].total_turnaround_time = 0;
		generate_arrival_times(&sellers[t]);
		pthread_create(&seller_threads[t], NULL, seller_thread, &sellers[t]);
		t++;
	}

	// Initialize 6 low-priced sellers (L)
	for (int i = 1; i <= 6; i++)
	{
		sellers[t].type = 'L';
		sellers[t].id = i; // For ID generation: L101, L102, etc.
		sellers[t].num_customers = N;
		sellers[t].queue = initQueue();
		sellers[t].next_arrival_index = 0;
		sellers[t].customer_count = 0;
		sellers[t].served = sellers[t].turned_away = 0;
		sellers[t].total_response_time = sellers[t].total_turnaround_time = 0;
		generate_arrival_times(&sellers[t]);
		pthread_create(&seller_threads[t], NULL, seller_thread, &sellers[t]);
		t++;
	}

	// Create the timer thread to drive simulation time.
	pthread_t timer;
	printf("\ncreating timer thread\n");
	pthread_create(&timer, NULL, timer_thread, NULL);

	printf("\nwaiting for threads\n");
	// Wait for all seller threads to finish.
	for (int i = 0; i < TOTAL_SELLERS; i++)
	{
		pthread_join(seller_threads[i], NULL);
		printf("Thread joined: %d", i);
		freeQueue(sellers[i].queue);
		free(sellers[i].arrival_times);
	}
	printf("waiting for timer");
	// Wait for the timer thread.
	pthread_join(timer, NULL);

	// Print summary statistics aggregated by seller type.
	int total_served_H = 0, total_served_M = 0, total_served_L = 0;
	int total_turned_H = 0, total_turned_M = 0, total_turned_L = 0;
	int total_response_H = 0, total_response_M = 0, total_response_L = 0;
	int total_turnaround_H = 0, total_turnaround_M = 0, total_turnaround_L = 0;

	// Seller index 0 is H; next 3 are M; next 6 are L.
	total_served_H = sellers[0].served;
	total_turned_H = sellers[0].turned_away;
	total_response_H = sellers[0].total_response_time;
	total_turnaround_H = sellers[0].total_turnaround_time;
	for (int i = 1; i <= 3; i++)
	{
		total_served_M += sellers[i].served;
		total_turned_M += sellers[i].turned_away;
		total_response_M += sellers[i].total_response_time;
		total_turnaround_M += sellers[i].total_turnaround_time;
	}
	for (int i = 4; i < TOTAL_SELLERS; i++)
	{
		total_served_L += sellers[i].served;
		total_turned_L += sellers[i].turned_away;
		total_response_L += sellers[i].total_response_time;
		total_turnaround_L += sellers[i].total_turnaround_time;
	}

	printf("=== Simulation Summary ===\n");
	if (total_served_H > 0)
		printf("High-priced (H): Served %d customers, Turned away %d, Average Response Time: %.2f, Average Turnaround Time: %.2f\n",
			   total_served_H, total_turned_H,
			   total_response_H / (float)total_served_H,
			   total_turnaround_H / (float)total_served_H);
	else
		printf("High-priced (H): Served 0 customers, Turned away %d\n", total_turned_H);

	if (total_served_M > 0)
		printf("Medium-priced (M): Served %d customers, Turned away %d, Average Response Time: %.2f, Average Turnaround Time: %.2f\n",
			   total_served_M, total_turned_M,
			   total_response_M / (float)total_served_M,
			   total_turnaround_M / (float)total_served_M);
	else
		printf("Medium-priced (M): Served 0 customers, Turned away %d\n", total_turned_M);

	if (total_served_L > 0)
		printf("Low-priced (L): Served %d customers, Turned away %d, Average Response Time: %.2f, Average Turnaround Time: %.2f\n",
			   total_served_L, total_turned_L,
			   total_response_L / (float)total_served_L,
			   total_turnaround_L / (float)total_served_L);
	else
		printf("Low-priced (L): Served 0 customers, Turned away %d\n", total_turned_L);

	return 0;
}
