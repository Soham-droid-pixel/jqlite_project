const express = require('express');
const cors = require('cors');
const { execFile } = require('child_process');
const fs = require('fs');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3000;

// Middleware
app.use(cors());
app.use(express.json({ limit: '10mb' }));
app.use(express.static('public'));

// Determine the path to jqlite executable
// Assumes jqlite.exe is in the parent directory of jqlite_webapp
const JQLITE_PATH = path.join(__dirname, '..', 'jqlite.exe');
const TEMP_FILE = path.join(__dirname, 'temp_input.json');

/**
 * API endpoint to execute jqlite queries
 */
app.post('/api/query', async (req, res) => {
    const { json_data, query_string } = req.body;

    // Validation
    if (!json_data || !query_string) {
        return res.status(400).json({
            error: 'Missing required fields: json_data and query_string'
        });
    }

    try {
        // Write JSON data to temporary file
        fs.writeFileSync(TEMP_FILE, json_data, 'utf8');

        // Execute jqlite
        execFile(JQLITE_PATH, [query_string, TEMP_FILE], {
            timeout: 5000, // 5 second timeout
            maxBuffer: 1024 * 1024 * 10 // 10MB buffer
        }, (error, stdout, stderr) => {
            // Clean up temporary file
            try {
                if (fs.existsSync(TEMP_FILE)) {
                    fs.unlinkSync(TEMP_FILE);
                }
            } catch (cleanupError) {
                console.error('Failed to delete temp file:', cleanupError);
            }

            // Handle execution errors
            if (error) {
                // Check if it's a timeout
                if (error.killed) {
                    return res.json({
                        error: 'Query execution timeout (exceeded 5 seconds)'
                    });
                }

                // If there's stderr output, that's likely the actual error message
                if (stderr && stderr.trim()) {
                    return res.json({
                        error: stderr.trim()
                    });
                }

                // Generic error
                return res.json({
                    error: `Execution error: ${error.message}`
                });
            }

            // Check for stderr output (warnings or errors)
            if (stderr && stderr.trim()) {
                return res.json({
                    error: stderr.trim()
                });
            }

            // Success - return the result
            res.json({
                result: stdout.trim()
            });
        });

    } catch (err) {
        // Clean up on exception
        try {
            if (fs.existsSync(TEMP_FILE)) {
                fs.unlinkSync(TEMP_FILE);
            }
        } catch (cleanupError) {
            console.error('Failed to delete temp file:', cleanupError);
        }

        res.status(500).json({
            error: `Server error: ${err.message}`
        });
    }
});

/**
 * Health check endpoint
 */
app.get('/api/health', (req, res) => {
    const jqliteExists = fs.existsSync(JQLITE_PATH);
    res.json({
        status: 'ok',
        jqlite_available: jqliteExists,
        jqlite_path: JQLITE_PATH
    });
});

// Start server
app.listen(PORT, () => {
    console.log(`\n╔════════════════════════════════════════════╗`);
    console.log(`║   🚀 jqlite Web Server Running!           ║`);
    console.log(`╚════════════════════════════════════════════╝\n`);
    console.log(`   📡 Server:    http://localhost:${PORT}`);
    console.log(`   🔧 jqlite:    ${JQLITE_PATH}`);
    console.log(`   ✓ Ready:      ${fs.existsSync(JQLITE_PATH) ? 'Yes' : 'No (jqlite.exe not found!)'}\n`);
});
