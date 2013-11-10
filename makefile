all: router_init routed_LS

router_init: router_init.c
		gcc -o router_init router_init.c

routed_LS: routed_LS.c 
		gcc -o routed_LS routed_LS.c

clean: 
		rm -rf *o routed_LS router_init
