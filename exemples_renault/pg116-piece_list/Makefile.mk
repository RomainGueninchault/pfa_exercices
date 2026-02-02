LDFLAGS+= # Les libs supplementaires
CFLAGS+= -Werror -Wextra -Wall -Wno-error=unused-parameter
DEPS=provided.h enonce.c
OBJS=provided.o given.o
MALLOC=sentinel
