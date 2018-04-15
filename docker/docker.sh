echo ========= Build docker image
docker build -t otus.lessons.30.01 .
echo ========= Execute kkmeans
docker run --rm -i otus.lessons.30.01 test.sh
docker cp centers.csv centers.csv
docker cp kmeans.csv kmeans.csv
docker cp kmeans.png kmeans.png
xdg-open kmeans.png
echo ========= Remove docker image
docker rmi otus.lessons.30.01