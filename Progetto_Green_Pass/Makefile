# Makefile Green Pass

# Build di tutti i client e server
build: build_central_server \
		build_assistant_server \
		build_vaccinated_server \
		build_vaccinated_client \
		build_administrator_client \
		build_reviser_client

# Build server Centrale
build_central_server:
	gcc central_server.c \
		lib/sockets_utility.c \
		lib/thread_utility.c \
		lib/file_utility.c \
		lib/date_utility.c \
		lib/green_pass_utility.c \
		lib/encryption_utility.c \
		-o ./output/server_centrale

# Build server assistente
build_assistant_server:
	gcc assistant_server.c \
		lib/sockets_utility.c \
		lib/thread_utility.c \
		lib/file_utility.c \
		lib/date_utility.c \
		lib/green_pass_utility.c \
		lib/encryption_utility.c \
		-o ./output/server_assistente

# Build server centro vaccinale
build_vaccinated_server:
	gcc vaccination_center_server.c \
		lib/thread_utility.c \
		lib/sockets_utility.c \
		lib/file_utility.c \
		lib/date_utility.c \
		lib/green_pass_utility.c \
		lib/encryption_utility.c \
		-o ./output/centro_vaccinale

# Build client vaccinale
build_vaccinated_client:
	gcc vaccinated_client.c \
		lib/sockets_utility.c \
		lib/menu_utility.c \
		lib/date_utility.c \
		lib/code_verification.c \
		lib/digits_utility.c \
		lib/file_utility.c \
		lib/buffer_utility.c \
		lib/green_pass_utility.c \
		lib/thread_utility.c \
		lib/encryption_utility.c \
		-o ./output/client_vaccinale
		

# Build client revisore
build_reviser_client:
	gcc reviser_client.c \
		lib/sockets_utility.c \
		lib/menu_utility.c \
		lib/date_utility.c \
		lib/code_verification.c \
		lib/digits_utility.c \
		lib/file_utility.c \
		lib/buffer_utility.c \
		lib/green_pass_utility.c \
		lib/thread_utility.c \
		lib/encryption_utility.c \
		-o ./output/client_reviser

# Build client amministratore
build_administrator_client:
	gcc administrator_client.c \
		lib/sockets_utility.c \
		lib/menu_utility.c \
		lib/date_utility.c \
		lib/code_verification.c \
		lib/digits_utility.c \
		lib/file_utility.c \
		lib/buffer_utility.c \
		lib/green_pass_utility.c \
		lib/thread_utility.c \
		lib/encryption_utility.c \
		-o output/client_administrator

# Run del server centrale
run_central_server:
	cd ./output &&\
		sudo ./server_centrale

# Run del server assisgente
run_assistant_server:
	cd ./output &&\
		sudo ./server_assistente

# Run del server vaccinale
run_vaccinated_server:
	cd ./output &&\
		sudo ./centro_vaccinale

# Run del client vaccinale
run_vaccinated_client:
	cd ./output &&\
		./client_vaccinale localhost

# Run del client revisore
run_reviser_client:
	cd ./output &&\
		./client_reviser localhost

# Run del client amministratore
run_administrator_client:
	cd ./output &&\
		./client_administrator localhost

clean:
	rm output/*
	
