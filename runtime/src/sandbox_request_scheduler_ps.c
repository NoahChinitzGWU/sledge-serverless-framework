#include <sandbox_request_scheduler.h>
#include "priority_queue.h"

// Local State
static struct priority_queue sandbox_request_scheduler_ps;

/**
 * Pushes a sandbox request to the global deque
 * @param sandbox_request
 * @returns pointer to request if added. NULL otherwise
 **/
static sandbox_request_t *
sandbox_request_scheduler_ps_add(void *sandbox_request_raw)
{
	int return_code = priority_queue_enqueue(&sandbox_request_scheduler_ps, sandbox_request_raw);

	return return_code == 0 ? sandbox_request_raw : NULL;
}

/**
 *
 * @returns A Sandbox Request or NULL
 **/
static sandbox_request_t *
sandbox_request_scheduler_ps_remove(void)
{
	return (sandbox_request_t *)priority_queue_dequeue(&sandbox_request_scheduler_ps);
}

unsigned long long int
sandbox_request_get_priority(void *element)
{
	sandbox_request_t *sandbox_request = (sandbox_request_t *)element;
	return sandbox_request->absolute_deadline;
};


/**
 *
 **/
void
sandbox_request_scheduler_ps_initialize()
{
	// Initialize local state
	priority_queue_initialize(&sandbox_request_scheduler_ps, sandbox_request_get_priority);

	// Register Function Pointers for Abstract Scheduling API
	sandbox_request_scheduler_config_t config = { .add    = sandbox_request_scheduler_ps_add,
		                                      .remove = sandbox_request_scheduler_ps_remove };

	sandbox_request_scheduler_initialize(&config);
}