build: worker.c master.c
	gcc -o worker worker.c -lm
	gcc -o master master.c

check: worker.c master.c
	./worker -x 2 -n 3
	./master --worker_path ./worker --wait_mechanism sequential -x 2 -n 12
	./master --worker_path ./worker --wait_mechanism select -x 2 -n 12
	./master --worker_path ./worker --wait_mechanism poll -x 2 -n 12
	./master --worker_path ./worker --wait_mechanism epoll -x 2 -n 12

clean: worker.c master.c
	rm -rf master worker

worker: worker.c
	gcc -o worker worker.c -lm

master: master.c
	gcc -o master master.c

sequential: worker master
	./master --worker_path ./worker --wait_mechanism sequential -x 2 -n 12

select: worker master
	./master --worker_path ./worker --wait_mechanism select -x 2 -n 12

poll: worker master
	./master --worker_path ./worker --wait_mechanism poll -x 2 -n 12

epoll: worker master
	./master --worker_path ./worker --wait_mechanism epoll -x 2 -n 12

