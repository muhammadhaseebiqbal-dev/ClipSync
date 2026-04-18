package com.ecosystem.client

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        // Creating a simple dynamic UI instead of XML layouts for extreme simplicity in setup
        val layout = android.widget.LinearLayout(this).apply {
            orientation = android.widget.LinearLayout.VERTICAL
            setPadding(64, 64, 64, 64)
        }

        val title = TextView(this).apply {
            text = "Ecosystem Clipboard Client\n"
            textSize = 24f
            setTypeface(null, android.graphics.Typeface.BOLD)
        }

        val instructions = TextView(this).apply {
            text = "Instruction:\n1. Open your Android Settings > Bluetooth\n" +
                   "2. Pair this phone with your Windows PC\n" +
                   "3. Once paired, click the Start button below to enable the transparent connection.\n\n"
            textSize = 16f
        }

        val statusText = TextView(this).apply {
            text = "Status: Waiting to start..."
            textSize = 18f
        }

        val startBtn = Button(this).apply {
            text = "Start Bluetooth Background Sync"
            setOnClickListener {
                checkPermissionsAndStart()
                statusText.text = "Status: Service Started (look at notifications)"
            }
        }

        layout.addView(title)
        layout.addView(instructions)
        layout.addView(statusText)
        layout.addView(startBtn)
        setContentView(layout)
    }

    private fun checkPermissionsAndStart() {
        val perms = mutableListOf<String>()
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            perms.add(Manifest.permission.BLUETOOTH_CONNECT)
            perms.add(Manifest.permission.BLUETOOTH_SCAN)
        }
        
        if (perms.any { ActivityCompat.checkSelfPermission(this, it) != PackageManager.PERMISSION_GRANTED }) {
            ActivityCompat.requestPermissions(this, perms.toTypedArray(), 100)
        } else {
            startClipboardService()
        }
    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == 100 && grantResults.all { it == PackageManager.PERMISSION_GRANTED }) {
            startClipboardService()
        }
    }

    private fun startClipboardService() {
        val intent = Intent(this, ClipboardBluetoothService::class.java)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            startForegroundService(intent)
        } else {
            startService(intent)
        }
    }
}
