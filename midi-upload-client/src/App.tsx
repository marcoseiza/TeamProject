import React from "react";

function App() {
  const [midiFile, setMidiFile] = React.useState<File>();
  const [arduinoIP, setArduinoIP] = React.useState<string>();

  const handleFileChange = React.useCallback(
    (e: React.ChangeEvent<HTMLInputElement>) => {
      const file = e.target.files?.item(0) || undefined;
      setMidiFile(file);
    },
    [setMidiFile]
  );

  const handleIPChange = React.useCallback(
    (e: React.ChangeEvent<HTMLInputElement>) => {
      setArduinoIP(e.target.value);
    },
    [setArduinoIP]
  );

  const handleUpload = React.useCallback(async () => {
    if (!midiFile || !arduinoIP) return;
    const data = new FormData();
    data.append("data", midiFile);
    await fetch(`http://${arduinoIP}/midi-upload`, {
      method: "POST",
      body: data,
      mode: "no-cors",
    });
  }, [midiFile, arduinoIP]);

  return (
    <div className="App">
      <label>Input Arduino Local IP</label>
      <span>Shown in Serial Monitor</span>
      <input
        type="text"
        name="Arduino IP"
        onChange={handleIPChange}
        className="input-ip"
        placeholder="Arduino IP"
      />
      <label>Upload Midi File</label>
      <input
        type="file"
        name="file"
        accept=".mid,.midi"
        onChange={handleFileChange}
        className="input-file"
      />
      <button type="button" onClick={handleUpload} className="upload">
        Upload
      </button>
    </div>
  );
}

export default App;
