user:
	make -f makefile_multi
home:
	make -f makefile_multi HOME_HOOK_ON=1
adhoc:
	make -f makefile_multi USE_ADHOC=1
mehome:
	make -f makefile_multi ME_ACTIVE=1 HOME_HOOK_ON=1
me:
	make -f makefile_multi ME_ACTIVE=1
mehomeadhoc:
	make -f makefile_multi ME_ACTIVE=1 HOME_HOOK_ON=1 USE_ADHOC=1
clean:
	make -f makefile_multi clean
