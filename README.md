# 실내 온습도 모니터링

ESP8266과 DHT11 센서에서 측정한 온도·습도를 React 웹 화면으로 확인하는 프로젝트입니다.

## 주요 기능

- 현재 온도·습도와 각 측정값 바로 아래의 기준값 표시
- 온도 또는 습도가 기준값 이상일 때 경고 표시
- LED의 ON/OFF 상태 표시
- ESP8266 연결 실패 안내 및 모바일 대응 화면

## 기준값과 동작

| 항목 | 설정 |
| --- | --- |
| 기준 온도 | 30℃ |
| 기준 습도 | 70% |
| 온도 경고 조건 | 30℃ 이상 |
| 습도 경고 조건 | 70% 이상 |
| 센서 측정 간격 | 2초 |
| 웹 상태 조회 간격 | 1초 |
| 정상 상태 LED | 1분 주기로 처음 15초 동안 켜짐 |
| 경고 상태 LED | 5초마다 ON/OFF 전환 |

온도와 습도 중 하나라도 기준값 이상이면 경고 상태가 됩니다. 웹 화면의 기준값은 ESP8266의 `/status` 응답에서 받아 표시하며, 수신 전에는 `--`로 표시합니다.

## 프로젝트 구성

```text
senser/
├── package.json                    # Sass 의존성
├── sketch_sep23b/
│   └── sketch_sep23b.ino            # 센서 측정, LED 제어, HTTP API
└── sensor-web/
    ├── src/App.jsx                 # 센서 상태 조회 및 화면
    ├── src/App.scss                # 화면 스타일
    └── src/index.scss              # 공통 스타일
```

## ESP8266 설정

1. Arduino IDE에서 `../sketch_sep23b/sketch_sep23b.ino`를 엽니다.
2. ESP8266 보드 지원과 스케치에서 사용하는 DHT 라이브러리를 준비합니다.
3. DHT11 데이터 핀을 `D7`, LED 제어 핀을 `D5`에 맞춰 연결합니다.
4. 스케치의 `ssid`와 `password`를 사용할 Wi-Fi 정보로 변경합니다.
5. 사용하는 보드와 포트를 선택하고 스케치를 업로드합니다.
6. 시리얼 모니터를 `115200` baud로 열어 ESP8266의 IP 주소를 확인합니다.

기준값은 스케치의 다음 상수에서 변경합니다.

```cpp
const float TEMP_LIMIT = 30.0;
const float HUM_LIMIT = 70.0;
```

변경 후에는 ESP8266에 스케치를 다시 업로드해야 실제 경고 기준과 웹 화면에 반영됩니다.

## 웹 실행

Node.js와 npm이 설치된 환경에서 프로젝트 최상위 폴더(`senser`)를 기준으로 실행합니다. 현재 Sass 의존성은 최상위 폴더에 있으므로 두 위치의 의존성을 모두 설치합니다.

```sh
npm install
cd sensor-web
npm install
```

`src/App.jsx`의 `ESP_URL`을 시리얼 모니터에서 확인한 주소로 설정합니다.

```js
const ESP_URL = "http://<ESP8266의 IP 주소>";
```

PC와 ESP8266이 서로 통신할 수 있는 네트워크에 연결된 상태에서 개발 서버를 실행합니다.

```sh
npm run dev
```

터미널에 표시되는 주소를 브라우저에서 엽니다.

## 빌드 및 코드 검사

`sensor-web` 폴더에서 실행합니다.

```sh
npm run build
npm run lint
npm run preview
```

빌드 결과는 `dist` 폴더에 생성됩니다. `preview`는 빌드 결과를 로컬에서 확인할 때 사용합니다.

## API

| 경로 | 설명 |
| --- | --- |
| `/` | 서버 동작 확인용 텍스트 |
| `/status` | 센서 측정값, 기준값, 경고 및 LED 상태 JSON |

`/status` 응답 예시:

```json
{
  "sensorOK": true,
  "temperature": 25.0,
  "humidity": 55.0,
  "tempAlert": false,
  "humidityAlert": false,
  "alert": false,
  "led": true,
  "tempLimit": 30.0,
  "humidityLimit": 70.0
}
```

센서를 읽지 못하면 `sensorOK`는 `false`, `temperature`와 `humidity`는 `null`입니다.

## 연결 문제 확인

- 연결 실패 안내가 나오면 ESP8266 전원, Wi-Fi 연결, `ESP_URL`을 확인합니다.
- 브라우저에서 `http://<ESP8266의 IP 주소>/status`에 접속해 응답을 확인합니다.
- 센서 데이터를 확인할 수 없으면 DHT11 연결과 시리얼 모니터 출력을 확인합니다.
- 기준값이 바뀌지 않으면 수정한 스케치를 보드에 다시 업로드했는지 확인합니다.
