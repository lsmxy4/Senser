import { useEffect, useState } from "react";
import "./App.scss";

// ESP8266의 IP 주소
// 시리얼 모니터에 나온 주소로 변경
const ESP_URL = "http://10.61.21.46";

function App() {
  const [sensorData, setSensorData] = useState({
    temperature: null,
    humidity: null,
    tempLimit: null,
    humidityLimit: null,
    tempAlert: false,
    humidityAlert: false,
    alert: false,
    led: false,
    sensorOK: false,
  });

  const [connectionError, setConnectionError] = useState(false);

  // ESP8266에서 센서 데이터 가져오기
  const getSensorData = async () => {
    try {
      const response = await fetch(`${ESP_URL}/status`);

      if (!response.ok) {
        throw new Error("ESP8266 응답 오류");
      }

      const data = await response.json();

      setSensorData(data);
      setConnectionError(false);
    } catch (error) {
      console.error("센서 데이터 가져오기 실패:", error);
      setConnectionError(true);
    }
  };

  // 페이지가 실행되면 센서 데이터 가져오기
  useEffect(() => {
    getSensorData();

    // 1초마다 ESP8266 상태 확인
    const interval = setInterval(() => {
      getSensorData();
    }, 1000);

    // 페이지 종료 시 반복 제거
    return () => clearInterval(interval);
  }, []);

  // 상태 메시지 결정
  const getStatusMessage = () => {
    if (!sensorData.sensorOK) {
      return "센서 데이터를 확인할 수 없습니다.";
    }

    if (sensorData.tempAlert && sensorData.humidityAlert) {
      return "온도와 습도가 기준치를 초과했습니다.";
    }

    if (sensorData.tempAlert) {
      return "온도가 기준치를 초과했습니다.";
    }

    if (sensorData.humidityAlert) {
      return "습도가 기준치를 초과했습니다.";
    }

    return "현재 온도와 습도가 정상입니다.";
  };

  return (
    <div className="app">
      <h1>실내 온습도 모니터링</h1>

      {/* ESP 연결 상태 */}
      {connectionError && (
        <div className="connection-error">
          ESP8266에 연결할 수 없습니다.
        </div>
      )}

      {/* 온도 / 습도 / LED */}
      <div className="sensor-container">
        <div className="sensor-card">
          <h2>현재 온도</h2>

          <p className="sensor-value">
            {sensorData.temperature !== null
              ? `${sensorData.temperature} ℃`
              : "-- ℃"}
          </p>

          <p className="sensor-limit">
            기준 온도: {sensorData.tempLimit ?? "--"} ℃
          </p>

          {sensorData.tempAlert && (
            <p className="warning-text">
              온도가 기준치를 초과했습니다.
            </p>
          )}
        </div>

        <div className="sensor-card">
          <h2>현재 습도</h2>

          <p className="sensor-value">
            {sensorData.humidity !== null
              ? `${sensorData.humidity} %`
              : "-- %"}
          </p>

          <p className="sensor-limit">
            기준 습도: {sensorData.humidityLimit ?? "--"} %
          </p>

          {sensorData.humidityAlert && (
            <p className="warning-text">
              습도가 기준치를 초과했습니다.
            </p>
          )}
        </div>

        <div className="sensor-card">
          <h2>LED 상태</h2>

          <p className="sensor-value">
            {sensorData.led ? "ON" : "OFF"}
          </p>

          <div
            className={
              sensorData.led
                ? "led-indicator led-on"
                : "led-indicator led-off"
            }
          ></div>
        </div>
      </div>

      {/* 전체 상태 */}
      <div
        className={
          sensorData.alert
            ? "status-box status-warning"
            : "status-box status-normal"
        }
      >
        <h2>현재 상태</h2>

        <p>{getStatusMessage()}</p>

        {sensorData.alert ? (
          <p>
            경고 상태입니다. LED가 5초 간격으로 깜빡입니다.
          </p>
        ) : (
          <p>
            정상 상태입니다. LED는 1분 주기로 15초 동안 켜집니다.
          </p>
        )}
      </div>
    </div>
  );
}

export default App;
