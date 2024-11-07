using Microsoft.Maui.Controls;
using MQTTnet;
using MQTTnet.Client;
using System;
using System.Text;

namespace BrokerMQTT
{
    public partial class MainPage : ContentPage
    {
        private IMqttClient _mqttClient;
        private float _temperature = 0;
        private float _humidity = 0;

        public MainPage()
        {
            InitializeComponent();
            InitializeMqttClient();
        }

        private async void InitializeMqttClient()
        {
            var factory = new MqttFactory();
            _mqttClient = factory.CreateMqttClient();

            var options = new MqttClientOptionsBuilder()
                .WithTcpServer("test.mosquitto.org")
                .Build();

            _mqttClient.ApplicationMessageReceivedAsync += e =>
            {
                var topic = e.ApplicationMessage.Topic;
                var payload = Encoding.UTF8.GetString(e.ApplicationMessage.Payload);

                if (topic == "/Markame/temp")
                {
                    _temperature = float.Parse(payload);
                    UpdateTemperatureSlider(_temperature);  // Atualiza o slider de temperatura
                }
                else if (topic == "/Markame/hum")
                {
                    _humidity = float.Parse(payload);
                    UpdateHumiditySlider(_humidity);  // Atualiza o slider de umidade
                }

                return Task.CompletedTask;
            };

            await _mqttClient.ConnectAsync(options);
            await _mqttClient.SubscribeAsync(new MqttTopicFilterBuilder().WithTopic("/Markame/temp").Build());
            await _mqttClient.SubscribeAsync(new MqttTopicFilterBuilder().WithTopic("/Markame/hum").Build());
        }

        private void UpdateTemperatureSlider(float temperature)
        {
            MainThread.BeginInvokeOnMainThread(() =>
            {
                temperatureSlider.Value = temperature / 100;  // Ajusta o valor para manter o ponto decimal
                temperatureLabel.Text = $"Temperatura: {temperatureSlider.Value:F2} °C";  // Exibe o valor formatado
            });
        }

        private void UpdateHumiditySlider(float humidity)
        {
            MainThread.BeginInvokeOnMainThread(() =>
            {
                humiditySlider.Value = humidity/10 ;  // Ajusta o valor para manter o ponto decimal
                humidityLabel.Text = $"Umidade: {humiditySlider.Value:F2} %";  // Exibe o valor formatado
            });
        }
    }
}
