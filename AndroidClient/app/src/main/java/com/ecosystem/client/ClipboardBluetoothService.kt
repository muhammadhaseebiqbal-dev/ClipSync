package com.ecosystem.client

import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothSocket
import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import android.util.Log
import androidx.core.app.NotificationCompat
import java.io.InputStream
import java.io.OutputStream
import java.util.UUID

class ClipboardBluetoothService : Service() {

    private val CHANNEL_ID = "EcosystemClipboardChannel"
    private var btSocket: BluetoothSocket? = null
    private var isRunning = true
    private val uiHandler = Handler(Looper.getMainLooper())

    override fun onBind(intent: Intent?): IBinder? = null

    override fun onCreate() {
        super.onCreate()
        createNotificationChannel()
        val notification = NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("Ecosystem Sync")
            .setContentText("Listening silently in the background...")
            .setSmallIcon(android.R.drawable.ic_menu_preferences)
            .setPriority(NotificationCompat.PRIORITY_LOW)
            .build()

        startForeground(1, notification)
        startBluetoothConnectionThread()
    }

    private fun createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val channel = NotificationChannel(
                CHANNEL_ID, "Clipboard Bluetooth Sync", NotificationManager.IMPORTANCE_LOW
            ).apply { description = "Keeps the Bluetooth connection alive." }
            getSystemService(NotificationManager::class.java)?.createNotificationChannel(channel)
        }
    }

    private fun startBluetoothConnectionThread() {
        Thread {
            val btManager = getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
            val btAdapter = btManager.adapter ?: return@Thread // No BT support

            while (isRunning) {
                try {
                    // Look through already-paired devices for your PC.
                    // This creates the true zero-hassle "paired-once, forever-connected" experience.
                    val pairedDevices = btAdapter.bondedDevices
                    if (pairedDevices.isNotEmpty()) {
                        for (device in pairedDevices) {
                            try {
                                // SPP UUID (Serial Port Profile). This acts like a virtual COM cable.
                                val uuid = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")
                                btSocket = device.createInsecureRfcommSocketToServiceRecord(uuid)
                                btSocket?.connect()

                                if (btSocket?.isConnected == true) {
                                    Log.d("EcoSync", "Connected to Desktop: ${device.name}")
                                    listenForDesktopClipboard(btSocket!!.inputStream)
                                    break // Broken out of loop if connection died
                                }
                            } catch (e: Exception) {
                                btSocket?.close()
                            }
                        }
                    }
                } catch (e: Exception) {}
                
                // If the desktop falls asleep or disconnects, wait 5 seconds and silently retry.
                Thread.sleep(5000)
            }
        }.start()
    }

    private fun listenForDesktopClipboard(inputStream: InputStream) {
        val buffer = ByteArray(4096)
        while (isRunning) {
            try {
                val bytesRead = inputStream.read(buffer)
                if (bytesRead == -1) break // Connection closed
                
                // We received text from the PC!
                val receivedText = String(buffer, 0, bytesRead).trim()
                Log.d("EcoSync", "Received from Desktop: $receivedText")
                setAndroidClipboard(receivedText)

            } catch (e: Exception) {
                break
            }
        }
    }

    private fun setAndroidClipboard(text: String) {
        // ClipboardManager must be touched on the UI thread internally in many Android versions
        uiHandler.post {
            val clipboard = getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
            val clip = ClipData.newPlainText("Ecosystem Sync", text)
            clipboard.setPrimaryClip(clip)
            Log.d("EcoSync", "Successfully synced Android Clipboard.")
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        isRunning = false
        btSocket?.close()
    }
}