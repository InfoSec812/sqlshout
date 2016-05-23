####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = sqlshout

## INCLUDES were found outside kdevelop specific part

sqlshout_SOURCES = reencode.c mysql.c main.c metadata.c utils.c network.c
sqlshout_LDADD   = -lc -lmysqlclient -lshout -lmp3lame -lmad -lpthread -lm


EXTRA_DIST = main.c mysql.c mysql.h reencode.c utils.c utils.h reencode.h network.h 

####### kdevelop will overwrite this part!!! (end)############
# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
sqlshout_LDFLAGS = $(all_libraries) 
                                                                                
