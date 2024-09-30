CFLAGS = -Wall -O2
CXXFLAGS = $(CFLAGS)

DRIVER_NAME = i2c_driver
OBJS = i2c_driver.o

all: $(DRIVER_NAME)

$(DRIVER_NAME): $(OBJS)
	$(CXX) -o $(DRIVER_NAME) $(OBJS) -lroot

clean:
	rm -f $(OBJS) $(DRIVER_NAME)

install:
	mkdir -p /boot/system/non-packaged/add-ons/kernel/drivers/bin/
	cp $(DRIVER_NAME) /boot/system/non-packaged/add-ons/kernel/drivers/bin/
	mkdir -p /dev/bus/i2c
