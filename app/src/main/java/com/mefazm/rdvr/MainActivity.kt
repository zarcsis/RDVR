package com.mefazm.rdvr

import android.content.Context
import com.mefazm.rdvr.databinding.ActivityMainBinding
import androidx.appcompat.app.AppCompatActivity
import androidx.core.widget.doAfterTextChanged
import android.os.Bundle
import android.text.Editable
import android.content.Intent
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.stringPreferencesKey
import androidx.datastore.preferences.preferencesDataStore
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        Log.i("onCreate()")
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding!!.root)

        val textWatcher = fun(_: Editable?) {
            binding!!.buttonConnect.isEnabled =
                binding!!.editTextHostname.text.isNotEmpty() &&
                        binding!!.editTextUsername.text.isNotEmpty() &&
                        binding!!.editTextPassword.text.isNotEmpty()
        }

        binding!!.editTextHostname.doAfterTextChanged(textWatcher)
        binding!!.editTextUsername.doAfterTextChanged(textWatcher)
        binding!!.editTextPassword.doAfterTextChanged(textWatcher)

        binding!!.buttonConnect.setOnClickListener {
            Log.i("Connect clicked.")
            startActivity(Intent(this, SessionActivity::class.java).apply {
                putExtra("hostname", binding!!.editTextHostname.text.toString())
                putExtra("username", binding!!.editTextUsername.text.toString())
                putExtra("password", binding!!.editTextPassword.text.toString())
            })
        }
    }

    override fun onResume() {
        Log.i("onResume()")
        super.onResume()
        CoroutineScope(Dispatchers.IO).launch {
            dataStore.data.collect { settings ->
                val hst = settings[hostnameKey] ?: ""
                val usr = settings[usernameKey] ?: ""
                val pwd = settings[passwordKey] ?: ""
                lifecycleScope.launch {
                    binding!!.editTextHostname.setText(hst)
                    binding!!.editTextUsername.setText(usr)
                    binding!!.editTextPassword.setText(pwd)
                }
            }
        }
    }

    override fun onPause() {
        Log.i("onPause()")
        super.onPause()
        var hst = binding!!.editTextHostname.text.toString()
        var usr = binding!!.editTextUsername.text.toString()
        var pwd = binding!!.editTextPassword.text.toString()
        CoroutineScope(Dispatchers.IO).launch {
            dataStore.edit { settings ->
                settings[hostnameKey] = hst
                settings[usernameKey] = usr
                settings[passwordKey] = pwd
            }
        }
    }

    private var binding: ActivityMainBinding? = null

    private companion object {
        private val hostnameKey = stringPreferencesKey("HOSTNAME")
        private val usernameKey = stringPreferencesKey("USERNAME")
        private val passwordKey = stringPreferencesKey("PASSWORD")
        private val Context.dataStore: DataStore<Preferences> by preferencesDataStore(name = "settings")
    }
}
