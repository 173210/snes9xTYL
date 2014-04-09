all: timestamp me

clean:
	make -f makefile_multi clean_snes

user: timestamp
	make -f makefile_multi

home: timestamp
	make -f makefile_multi HOME_HOOK_ON=1

adhoc: timestamp
	make -f makefile_multi USE_ADHOC=1
mehome: timestamp
	make -f makefile_multi ME_ACTIVE=1 HOME_HOOK_ON=1

me:
	make -f makefile_multi ME_ACTIVE=1

mehomeadhoc: timestamp
	make -f makefile_multi ME_ACTIVE=1 HOME_HOOK_ON=1 USE_ADHOC=1

timestamp:
	sh make_timestamp
	
