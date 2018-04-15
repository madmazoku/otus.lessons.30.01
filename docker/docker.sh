echo ========= Build docker image
docker build -t otus.lessons.30.01 .
echo ========= Execute kkmeans
docker run --name otus.lessons.30.01 otus.lessons.30.01 ./test.sh
docker cp otus.lessons.30.01:/centers.csv ./centers.csv
docker cp otus.lessons.30.01:/kmeans.csv ./kmeans.csv
docker cp otus.lessons.30.01:/kmeans.png ./kmeans.png
docker rm otus.lessons.30.01
xdg-open kmeans.png
echo ========= Remove docker image
docker rmi otus.lessons.30.01