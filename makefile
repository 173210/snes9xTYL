all: timestamp me

clean:
	make -f makefile_me clean
	make -f makefile_user clean

user: timestamp
	make -f makefile_user
	
home: timestamp
	make -f makefile_user_homehook
	
mehome: timestamp
	make -f makefile_me_homehook

me:
	make -f makefile_me
	
timestamp:
	rem sh make_timestamp
	
