user: makefile_multi
	$(MAKE) -f $<
home: makefile_multi
	$(MAKE) -f $< HOME_HOOK_ON=1
adhoc: makefile_multi
	$(MAKE) -f $< USE_ADHOC=1
mehome: makefile_multi
	$(MAKE) -f $< ME_ACTIVE=1 HOME_HOOK_ON=1
me: makefile_multi
	$(MAKE) -f $< ME_ACTIVE=1
mehomeadhoc: makefile_multi
	$(MAKE) -f $< ME_ACTIVE=1 HOME_HOOK_ON=1 USE_ADHOC=1
clean: makefile_multi
	$(MAKE) -f $< clean
