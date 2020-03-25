default:
	$(MAKE) -C app

run:
	$(MAKE) -C app run

clean:
	$(MAKE) -C app clean

docker-image:
	cd docker && docker build -t monolinux-raspberry-pi-3 .

docker-image-tag-and-push:
	docker tag monolinux-raspberry-pi-3:latest eerimoq/monolinux-raspberry-pi-3:$(TAG)
	docker push eerimoq/monolinux-raspberry-pi-3:$(TAG)
