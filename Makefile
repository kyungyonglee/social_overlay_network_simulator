CC=g++
CFLAGS=-c -Wall -g
LDFLAGS=
SOURCES=son_test.cpp SocialOverlayNetwork.cpp son_friend_select.cpp son_msg_dist.cpp son_routing.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=son_test

all: $(SOURCES) $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(EXECUTABLE)

son_test.o: $(SOURCES) 
	$(CC) $(CFLAGS) son_test.cpp

SocialOverlayNetwork.o : SocialOverlayNetwork.cpp SocialOverlayNetwork.h
	$(CC) $(CFLAGS) SocialOverlayNetwork.cpp

son_friend_select.o : son_friend_select.cpp son_friend_select.h
	$(CC) $(CFLAGS) son_friend_select.cpp

son_msg_dist.o : son_msg_dist.cpp son_msg_dist.h
	$(CC) $(CFLAGS) son_msg_dist.cpp

son_routing.o : son_routing.cpp son_routing.h
	$(CC) $(CFLAGS) son_routing.cpp

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(EXECCUTABLE)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $(EXECUTABLE)

clean:
	rm -rf *o son_test

