docker compose is required for the following steps.

1. create neccesary local files:

mkdir -p ~/.mytb-data && sudo chown -R 799:799 ~/.mytb-data
mkdir -p ~/.mytb-logs && sudo chown -R 799:799 ~/.mytb-logs


2. download and start the thingsboard instance

docker-compose pull
docker-compose up

3. open http://{your-host-ip}:8080 in the browser:

login: tenant@thingsboard.org / tenant

4. create device using MQTT basic credentials and the clients clientID

5. import the two rules and the dashboard
