echo ========= Build docker image
docker build -t otus.lessons.30.01 .
echo ========= Execute kkmeans
docker run --rm -i otus.lessons.30.01 kkmeans -v
echo ========= Remove docker image
docker rmi otus.lessons.30.01