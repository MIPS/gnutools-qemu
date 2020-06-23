#ifndef BBV_ROUTINES_H
#define BBV_ROUTINES_H
/* vmw */

void do_dump_pc(uint32_t bb);
extern char bbv_file_name[PATH_MAX];
extern int bbv_interval_size;
#if !defined(TARGET_ARM)
void helper_dump_pc(uint32_t bb);
#endif

void gen_helper_dump_pc(uint32_t bb);


#define MAX_BBS 100000

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

   if (!strcmp (bbv_file_name, ""))
     return;
   
   if (bb==0xffffffff) {
      if (bbv_file!=NULL) {
	 long long total;
	 total=((long long)intervals*bbv_interval_size)+(long long)total_count;
	 fprintf(bbv_file,"# Total count : %lld\n",total);
#if defined(TARGET_I386) || defined (TARGET_X86_64)
	 fprintf(bbv_file,"# Rep count   : %lld\n",total_reps);
#endif          
	 fclose(bbv_file);
	 return;
      }
   }
   
   if (bbv_file==NULL) {
      bbv_file=fopen(bbv_file_name,"w");
      if (bbv_file==NULL) {
	printf("Error!  Could not open file %s\n",bbv_file_name);
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
        
   if (total_count>=bbv_interval_size) {
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
void helper_dump_pc(uint32_t bb) {
   do_dump_pc(bb);
}

#endif /* BBV_ROUTINES_H */
