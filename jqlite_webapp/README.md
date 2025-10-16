# jqlite Web UI

A beautiful, modern web interface for the jqlite JSON query tool.

## 🎨 Features

- **Monaco Editor Integration** - Professional code editor with syntax highlighting
- **Real-time Query Execution** - Execute jqlite queries instantly
- **Modern Dark Theme** - Beautiful, easy-on-the-eyes interface
- **Interactive Examples** - Quick-start query examples
- **Error Handling** - Clear error messages and validation
- **Responsive Design** - Works on desktop and mobile devices

## 🚀 Quick Start

### Prerequisites

- Node.js (v14 or higher)
- jqlite.exe compiled and in the parent directory

### Installation

1. Navigate to the webapp directory:
```bash
cd jqlite_webapp
```

2. Install dependencies:
```bash
npm install
```

3. Start the server:
```bash
npm start
```

4. Open your browser and visit:
```
http://localhost:3000
```

## 📁 Project Structure

```
jqlite_webapp/
├── public/
│   ├── index.html      # Main HTML file
│   └── style.css       # Styles and theme
├── server.js           # Express backend
├── package.json        # Dependencies
└── README.md          # This file
```

## 🔧 How It Works

1. **Frontend**: The user enters JSON data and a query string in the Monaco editors
2. **API Call**: The frontend sends a POST request to `/api/query` with the data
3. **Backend**: Express server writes JSON to a temp file and executes `jqlite.exe`
4. **Response**: The result or error is sent back and displayed in the result editor

## 💡 Example Queries

- **Basic field access**: `.name`
- **Array indexing**: `.posts[0]`
- **Pipe operations**: `.posts | [0] | .title`
- **Filtering**: `.posts | .[] | select(.likes > 50)`
- **Array slicing**: `.posts[0:2]`
- **Array iteration**: `.posts | .[]`

## 🎯 API Endpoints

### POST `/api/query`
Execute a jqlite query on JSON data.

**Request Body:**
```json
{
  "json_data": "{ \"name\": \"John\" }",
  "query_string": ".name"
}
```

**Response (Success):**
```json
{
  "result": "\"John\""
}
```

**Response (Error):**
```json
{
  "error": "Error message from jqlite"
}
```

### GET `/api/health`
Check server and jqlite availability.

**Response:**
```json
{
  "status": "ok",
  "jqlite_available": true,
  "jqlite_path": "C:\\path\\to\\jqlite.exe"
}
```

## 🛠️ Development

To run in development mode with auto-restart:

```bash
npm run dev
```

## 📝 Notes

- The server looks for `jqlite.exe` in the parent directory of `jqlite_webapp/`
- Temporary files are created and cleaned up automatically
- Query execution has a 5-second timeout
- Maximum JSON size is 10MB

## 🎨 Customization

You can customize the theme by editing the CSS variables in `public/style.css`:

```css
:root {
    --accent-primary: #007acc;  /* Change primary color */
    --bg-primary: #1e1e1e;      /* Change background */
    /* ... more variables ... */
}
```

## 🐛 Troubleshooting

**Server won't start:**
- Make sure Node.js is installed: `node --version`
- Check if port 3000 is available

**jqlite.exe not found:**
- Ensure jqlite.exe is in the parent directory
- Check the path in server.js `JQLITE_PATH` variable

**Queries not working:**
- Verify jqlite.exe is executable
- Check the console for error messages
- Visit `/api/health` to verify server status

## 📄 License

MIT License - Same as jqlite project
