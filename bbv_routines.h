/* vmw */

void do_dump_pc(uint32_t bb);

#if !defined(TARGET_ARM)
void helper_dump_pc(uint32_t bb);
#endif

void gen_helper_dump_pc(uint32_t bb);


#define MAX_BBS 100000
#define INTERVAL_SIZE 100000000 /* 100 million */

void do_dump_pc(unsigned int bb) {

   static unsigned long total_count=0,intervals=0;
   static int bbvs[MAX_BBS];
   int i;
   static FILE *bbv_file=NULL;
   
#if defined(TARGET_I386) || defined (TARGET_X86_64)
   static int rep_count=0;
   int rep;
   static long long total_reps=0;
#endif   

   if (bb==0xffffffff) {
      if (bbv_file!=NULL) {
	 long long total;
	 total=((long long)intervals*INTERVAL_SIZE)+(long long)total_count;
	 fprintf(bbv_file,"# Total count : %lld\n",total);
#if defined(TARGET_I386) || defined (TARGET_X86_64)
	 fprintf(bbv_file,"# Rep count   : %lld\n",total_reps);
#endif          
	 fclose(bbv_file);
	 return;
      }
   }
   
   if (bbv_file==NULL) {
      bbv_file=fopen("qemusim.bbv","w");
      if (bbv_file==NULL) {
	 printf("Error!  Could not open file %s\n","qemusim.bbv");
	 exit(-1);
      }         
   }
   
#if defined(TARGET_I386) || defined (TARGET_X86_64)
   rep=bb&0x80000000;
   bb   &=0x7fffffff;
#endif   
   
   if (bb>MAX_BBS) {
      printf("Error!  Not enough BBS %d\n",bb);
      exit(-1);
   }
      
#if defined(TARGET_I386) || defined (TARGET_X86_64)
   
   if (rep) {
      rep_count++;
      total_reps++;
      return;
   }
      
   if ((rep_count) && (!rep)) {
      rep_count=0;
      /* count all reps as one instruction (as per docs) */
      /* this makes things match perf-ctr results        */
      total_count++;
      bbvs[bb]++;
   }     
#endif   

   total_count++;
   bbvs[bb]++;
        
   if (total_count>=INTERVAL_SIZE) {
      intervals++;
      fprintf(bbv_file,"T");
      for(i=0;i<MAX_BBS;i++) {
	 if (bbvs[i]) {
	    /* simpoint can't handle a basic block starting at zero? */
	    fprintf(bbv_file,":%d:%d ",i+1,bbvs[i]);
	 }
      }
      fprintf(bbv_file,"\n");
	
      /* clear the stats */
      total_count=0;
      for(i=0;i<MAX_BBS;i++) {
	 bbvs[i]=0;
      }
   }
}

/* grrr, why is this needed on x86 */ 
void helper_dump_pc(unsigned int bb) {
   do_dump_pc(bb);
}
