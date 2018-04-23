	.text

/* Switch from current_thread to next_thread. Make next_thread
 * the current_thread, and set next_thread to 0.
 * Use eax as a temporary register, which should be caller saved.
 */
	.globl thread_switch
thread_switch:
	/* YOUR CODE HERE */

	pushal						 /* all xv6 registers save where esp was */
	movl current_thread,%eax     /*address in curr_thread copied to eax*/
	movl %esp,(%eax)			 /* save esp to the address loc in %eax */
	movl next_thread,%eax					
	movl (%eax),%esp   			 /*next_thread address vali value ko esp mein daalna */
	movl %eax,current_thread 	 /*copy pointer of next_thread in curr_thread */
	popal						 /* esp vali position se 32 registers nikaal kr xv6 mein daalna */			
	movl $0,next_thread		 	 /* pointer ko zero kr do */						
	ret							 /* pop return address from stack */
