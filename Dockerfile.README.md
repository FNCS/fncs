# FNCS Dockerfile Info

## Starting using docker-compose

 1.  Edit docker-compose.yml for number of federates to be required.

````
	# Start in background.
	sudo docker-compose up -d

````

## Running "official" fncs image
````
	# Pull the latest image to your local registry
	sudo docker pull fncs/fncs

	# Make a place for output on the local filesystem
	sudo mkdir /var/data

	# Run fncs with 3 federates able to connect
	# -v mounts a volume from the local file system /var/data to the container /fncs directory
	# -t tag to run the latest official fncs image from
	# -d runs image in the background
	# -p mapps ports from host port -> container port
	sudo docker run -d -t fncs/fncs -p 5570:5570 -v /var/data:/fncs/log fncs_broker 2
````

### Verifying Running broker
````
	sudo lsof -i |grep 5570

	# And / Or
	cat /var/data/fncs.log
````


## Building image

````
	# Creates an image tagged fncs. ()
    sudo docker -t fncs build .
````

## Running fncs
````
	# Run fncs with 3 federates able to connect
	sudo docker run -p5570:5570 -t fncs fncs_broker 3 
````