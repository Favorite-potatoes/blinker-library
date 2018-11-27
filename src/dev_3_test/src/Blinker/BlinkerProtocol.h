#ifndef BLINKER_PROTOCOL_H
#define BLINKER_PROTOCOL_H

#include "Blinker/BlinkerApi.h"

#if defined(ESP8266)
    #include <ESP8266WiFi.h>
#elif defined(ESP32)
    #include <WiFi.h>
#endif

template <class Transp>
class BlinkerProtocol : public BlinkerApi< BlinkerProtocol<Transp> >
{
    friend class BlinkerApi< BlinkerProtocol<Transp> >;

    typedef BlinkerApi< BlinkerProtocol<Transp> > BApi;

    public :
        enum _blinker_state_t
        {
            CONNECTING,
            CONNECTED,
            DISCONNECTED
        };

        BlinkerProtocol(Transp& transp)
            : BApi()
            , conn(transp)
            , state(CONNECTING)
            , isFresh(false)
            , isAvail(false)
            , availState(false)
            , canParse(false)
        {}

        bool connect(uint32_t timeout = BLINKER_STREAM_TIMEOUT);
        bool connected()    { return state == CONNECTED; }
        void disconnect()   { conn.disconnect(); state = DISCONNECTED; }
        bool available()    { if (availState) {availState = false; return true;} else return false; }
        String readString();

        template <typename T>
        void print(T n)     { _print("\""  + STRING_format(n)+ "\""); }
        void print()        { _print("\"\""); }
        
        template <typename T>
        void println(T n)   { print(n); }
        void println()      { print(); }

        template <typename T1, typename T2, typename T3>
        void print(T1 n1, T2 n2, T3 n3);
        
        template <typename T1, typename T2, typename T3>
        void println(T1 n1, T2 n2, T3 n3)   { print(n1, n2, n3); }

        template <typename T1>
        void printArray(T1 n1, const String &s2);

        template <typename T1>
        void printObject(T1 n1, const String &s2);

        template <typename T1>
        void print(T1 n1, const String &s2);
        template <typename T1>
        void print(T1 n1, const char str2[]);
        template <typename T1>
        void print(T1 n1, char c);
        template <typename T1>
        void print(T1 n1, unsigned char b);
        template <typename T1>
        void print(T1 n1, int n);
        template <typename T1>
        void print(T1 n1, unsigned int n);
        template <typename T1>
        void print(T1 n1, long n);
        template <typename T1>
        void print(T1 n1, unsigned long n);
        template <typename T1>
        void print(T1 n1, double n);

        template <typename T1>
        void println(T1 n1, const String &s2)   { print(n1, s2); }
        template <typename T1>
        void println(T1 n1, const char str2[])  { print(n1, str2); }
        template <typename T1>
        void println(T1 n1, char c)             { print(n1, c); }
        template <typename T1>
        void println(T1 n1, unsigned char b)    { print(n1, b); }
        template <typename T1>
        void println(T1 n1, int n)              { print(n1, n); }
        template <typename T1>
        void println(T1 n1, unsigned int n)     { print(n1, n); }        
        template <typename T1>
        void println(T1 n1, long n)             { print(n1, n); }        
        template <typename T1>
        void println(T1 n1, unsigned long n)    { print(n1, n); }        
        template <typename T1>
        void println(T1 n1, double n)           { print(n1, n); }

        template <typename T>
        void notify(T n);

        void checkState(bool state = true)      { isCheck = state; }

        void flush();

        void run();

    private :
        #if defined(BLINKER_ARDUINOJSON)
            void autoFormatData(const String & key, const String & jsonValue);
        #else
            void autoFormatData(String & data);
        #endif
        void checkFormat();
        bool checkAvail();
        bool checkAliAvail() { return conn.aligenieAvail(); }
        char* dataParse()   { if (canParse) return conn.lastRead(); else return ""; }
        char* lastRead()    { return conn.lastRead(); }
        void isParsed()     { flush(); }
        bool parseState()   { return canParse; }
        void printNow();
        void checkAutoFormat();

    protected :
        Transp&             conn;
        _blinker_state_t    state;
        bool                isFresh;
        bool                isAvail;
        bool                availState;
        bool                canParse;
        bool                autoFormat = false;
        bool                isCheck = true;
        uint32_t            autoFormatFreshTime;
        char*               _sendBuf;

        bool            _isInit = false;
        uint8_t         _disconnectCount = 0;
        uint32_t        _disFreshTime = 0;
        uint32_t        _disconnectTime = 0;
        uint32_t        _refreshTime = 0;
        uint32_t        _reconTime = 0;

        void _print(char * n, bool needParse = true, bool needCheckLength = true);
        void begin();
        
};

template <class Transp>
bool BlinkerProtocol<Transp>::connect(uint32_t timeout)
{
    state = CONNECTING;

    uint32_t startTime = millis();
    while ( (state != CONNECTED) && \
        (millis() - startTime) < timeout )
    { run(); }

    return state == CONNECTED;
}

template <class Transp>
String BlinkerProtocol<Transp>::readString()
{
    if (isFresh)
    {
        isFresh = false;
        // char* r_data = (char*)malloc(strlen(conn.lastRead())*sizeof(char));
        // strcpy(r_data, conn.lastRead());

        String r_data = conn.lastRead();

        flush();
        return r_data;
    }
    else {
        return "";
    }
}

template <class Transp> template <typename T1, typename T2, typename T3>
void BlinkerProtocol<Transp>::print(T1 n1, T2 n2, T3 n3)
{
    String _msg = BLINKER_F("\"");
    _msg += STRING_format(n1);
    _msg += BLINKER_F("\":\"");
    _msg += STRING_format(n2);
    _msg += BLINKER_CMD_INTERSPACE;
    _msg += STRING_format(n3);
    _msg += BLINKER_F("\"");

    checkFormat();

    #if defined(BLINKER_ARDUINOJSON)
        autoFormatData(STRING_format(n1), _msg);
    #else
        autoFormatData(_msg);
    #endif

    autoFormatFreshTime = millis();
}

template <class Transp> template <typename T1>
void BlinkerProtocol<Transp>::printArray(T1 n1, const String &s2)
{
    String _msg = BLINKER_F("\"");
    _msg += STRING_format(n1);
    _msg += BLINKER_F("\":");
    _msg += s2;
    
    checkFormat();

    #if defined(BLINKER_ARDUINOJSON)
        autoFormatData(STRING_format(n1), _msg);
    #else
        autoFormatData(_msg);
    #endif

    autoFormatFreshTime = millis();
}

template <class Transp> template <typename T1>
void BlinkerProtocol<Transp>::printObject(T1 n1, const String &s2)
{
    String _msg = BLINKER_F("\"");
    _msg += STRING_format(n1);
    _msg += BLINKER_F("\":");
    _msg += s2;
    
    checkFormat();

    #if defined(BLINKER_ARDUINOJSON)
        autoFormatData(STRING_format(n1), _msg);
    #else
        autoFormatData(_msg);
    #endif

    autoFormatFreshTime = millis();
}

template <class Transp> template <typename T1>
void BlinkerProtocol<Transp>::print(T1 n1, const String &s2)
{
    String _msg = BLINKER_F("\"");
    _msg += STRING_format(n1);
    _msg += BLINKER_F("\":\"");
    _msg += s2;
    _msg += BLINKER_F("\"");

    checkFormat();

    #if defined(BLINKER_ARDUINOJSON)
        autoFormatData(STRING_format(n1), _msg);
    #else
        autoFormatData(_msg);
    #endif

    autoFormatFreshTime = millis();
}

template <class Transp> template <typename T1>
void BlinkerProtocol<Transp>::print(T1 n1, const char str2[])
{
    String _msg = BLINKER_F("\"");
    _msg += STRING_format(n1);
    _msg += BLINKER_F("\":\"");
    _msg += STRING_format(str2);
    _msg += BLINKER_F("\"");

    checkFormat();

    #if defined(BLINKER_ARDUINOJSON)
        autoFormatData(STRING_format(n1), _msg);
    #else
        autoFormatData(_msg);
    #endif

    autoFormatFreshTime = millis();
}

template <class Transp> template <typename T1>
void BlinkerProtocol<Transp>::print(T1 n1, char c)
{
    String _msg = BLINKER_F("\"");
    _msg += STRING_format(n1);
    _msg += BLINKER_F("\":");
    _msg += STRING_format(c);

    checkFormat();

    #if defined(BLINKER_ARDUINOJSON)
        autoFormatData(STRING_format(n1), _msg);
    #else
        autoFormatData(_msg);
    #endif

    autoFormatFreshTime = millis();
}

template <class Transp> template <typename T1>
void BlinkerProtocol<Transp>::print(T1 n1, unsigned char b)
{
    String _msg = BLINKER_F("\"");
    _msg += STRING_format(n1);
    _msg += BLINKER_F("\":");
    _msg += STRING_format(b);

    checkFormat();

    #if defined(BLINKER_ARDUINOJSON)
        autoFormatData(STRING_format(n1), _msg);
    #else
        autoFormatData(_msg);
    #endif

    autoFormatFreshTime = millis();
}

template <class Transp> template <typename T1>
void BlinkerProtocol<Transp>::print(T1 n1, int n)
{
    String _msg = BLINKER_F("\"");
    _msg += STRING_format(n1);
    _msg += BLINKER_F("\":");
    _msg + STRING_format(n);

    checkFormat();

    #if defined(BLINKER_ARDUINOJSON)
        autoFormatData(STRING_format(n1), _msg);
    #else
        autoFormatData(_msg);
    #endif

    autoFormatFreshTime = millis();        
}

template <class Transp> template <typename T1>
void BlinkerProtocol<Transp>::print(T1 n1, unsigned int n)
{
    String _msg = BLINKER_F("\"");
    _msg += STRING_format(n1);
    _msg += BLINKER_F("\":");
    _msg + STRING_format(n);

    checkFormat();

    #if defined(BLINKER_ARDUINOJSON)
        autoFormatData(STRING_format(n1), _msg);
    #else
        autoFormatData(_msg);
    #endif

    autoFormatFreshTime = millis();
}

template <class Transp> template <typename T1>
void BlinkerProtocol<Transp>::print(T1 n1, long n)
{
    String _msg = BLINKER_F("\"");
    _msg += STRING_format(n1);
    _msg += BLINKER_F("\":");
    _msg + STRING_format(n);

    checkFormat();

    #if defined(BLINKER_ARDUINOJSON)
        autoFormatData(STRING_format(n1), _msg);
    #else
        autoFormatData(_msg);
    #endif

    autoFormatFreshTime = millis();
}

template <class Transp> template <typename T1>
void BlinkerProtocol<Transp>::print(T1 n1, unsigned long n)
{
    String _msg = BLINKER_F("\"");
    _msg += STRING_format(n1);
    _msg += BLINKER_F("\":");
    _msg + STRING_format(n);

    checkFormat();

    #if defined(BLINKER_ARDUINOJSON)
        autoFormatData(STRING_format(n1), _msg);
    #else
        autoFormatData(_msg);
    #endif

    autoFormatFreshTime = millis();
}

template <class Transp> template <typename T1>
void BlinkerProtocol<Transp>::print(T1 n1, double n)
{
    String _msg = BLINKER_F("\"");
    _msg += STRING_format(n1);
    _msg += BLINKER_F("\":");
    _msg += STRING_format(n);

    checkFormat();

    #if defined(BLINKER_ARDUINOJSON)
        autoFormatData(STRING_format(n1), _msg);
    #else
        autoFormatData(_msg);
    #endif

    autoFormatFreshTime = millis();
}

template <class Transp> template <typename T>
void BlinkerProtocol<Transp>::notify(T n)
{
    String _msg = BLINKER_F("\"");
    _msg += STRING_format(BLINKER_CMD_NOTICE);
    _msg += BLINKER_F("\":\"");
    _msg += STRING_format(n);
    _msg += BLINKER_F("\"");
    
    checkFormat();

    #if defined(BLINKER_ARDUINOJSON)
        autoFormatData(STRING_format(BLINKER_CMD_NOTICE), _msg);
    #else
        autoFormatData(_msg);
    #endif

    autoFormatFreshTime = millis();
}

template <class Transp>
void BlinkerProtocol<Transp>::flush()
{ 
    conn.flush(); 
    isFresh = false; availState = false; 
    canParse = false; isAvail = false;
}

#if defined(BLINKER_ARDUINOJSON)
    template <class Transp>
    void BlinkerProtocol<Transp>::autoFormatData(const String & key, const String & jsonValue)
    {
        BLINKER_LOG_ALL(BLINKER_F("autoFormatData key: "), key, \
                        BLINKER_F(", json: "), jsonValue);

        String _data;

        if (STRING_contains_string(STRING_format(_sendBuf), key))
        {

            DynamicJsonBuffer jsonSendBuffer;                

            if (strlen(_sendBuf)) {
                BLINKER_LOG_ALL(BLINKER_F("add"));

                JsonObject& root = jsonSendBuffer.parseObject(STRING_format(_sendBuf));

                if (root.containsKey(key)) {
                    root.remove(key);
                }
                root.printTo(_data);

                _data = _data.substring(0, _data.length() - 1);

                _data += BLINKER_F(",");
                _data += jsonValue;
                _data += BLINKER_F("}");
            }
            else {
                BLINKER_LOG_ALL(BLINKER_F("new"));
                
                _data = BLINKER_F("{");
                _data += jsonValue;
                _data += BLINKER_F("}");
            }
        }
        else {
            _data = STRING_format(_sendBuf);

            if (_data.length())
            {
                BLINKER_LOG_ALL(BLINKER_F("add."));

                _data = _data.substring(0, _data.length() - 1);

                _data += BLINKER_F(",");
                _data += jsonValue;
                _data += BLINKER_F("}");
            }
            else {
                BLINKER_LOG_ALL(BLINKER_F("new."));
                
                _data = BLINKER_F("{");
                _data += jsonValue;
                _data += BLINKER_F("}");
            }
        }

        if (strlen(_sendBuf) > BLINKER_MAX_SEND_SIZE)
        {
            BLINKER_ERR_LOG(BLINKER_F("FORMAT DATA SIZE IS MAX THAN LIMIT"));
            return;
        }

        strcpy(_sendBuf, _data.c_str());
    }
#else
    template <class Transp>
    void BlinkerProtocol<Transp>::autoFormatData(String & data)
    {
        BLINKER_LOG_ALL(BLINKER_F("autoFormatData data: "), data);
        BLINKER_LOG_ALL(BLINKER_F("strlen(_sendBuf): "), strlen(_sendBuf));
        BLINKER_LOG_ALL(BLINKER_F("data.length(): "), data.length());

        if ((strlen(_sendBuf) + data.length()) >= BLINKER_MAX_SEND_SIZE)
        {
            BLINKER_ERR_LOG(BLINKER_F("FORMAT DATA SIZE IS MAX THAN LIMIT"));
            return;
        }

        if (strlen(_sendBuf) > 0) {
            data = "," + data;
            strcat(_sendBuf, data.c_str());
        }
        else {
            data = "{" + data;
            strcpy(_sendBuf, data.c_str());
        }
    }
#endif

template <class Transp>
void BlinkerProtocol<Transp>::checkFormat()
{
    if (!autoFormat)
    {
        autoFormat = true;
        _sendBuf = (char*)malloc(BLINKER_MAX_SEND_SIZE*sizeof(char));
        memset(_sendBuf, '\0', BLINKER_MAX_SEND_SIZE);
    }
}

template <class Transp>
bool BlinkerProtocol<Transp>::checkAvail()
{
    flush();

    isAvail = conn.available();
    if (isAvail)
    {
        isFresh = true;
        canParse = true;
        availState = true;
    }
    return isAvail;
}

template <class Transp>
void BlinkerProtocol<Transp>::printNow()
{
    if (strlen(_sendBuf) && autoFormat)
    {
        #if defined(BLINKER_ARDUINOJSON)
            _print(_sendBuf);
        #else
            strcat(_sendBuf, "}");
            _print(_sendBuf);
        #endif

        free(_sendBuf);
        autoFormat = false;
    }
}

template <class Transp>
void BlinkerProtocol<Transp>::checkAutoFormat()
{
    if (autoFormat)
    {
        if ((millis() - autoFormatFreshTime) >= BLINKER_MSG_AUTOFORMAT_TIMEOUT)
        {
            if (strlen(_sendBuf))
            {
                #if defined(BLINKER_ARDUINOJSON)
                    _print(_sendBuf);
                #else
                    strcat(_sendBuf, "}");
                    _print(_sendBuf);
                #endif
            }
            free(_sendBuf);
            autoFormat = false;
        }
    }
}

template <class Transp>
void BlinkerProtocol<Transp>::_print(char * n, bool needParse, bool needCheckLength)
{
    BLINKER_LOG_ALL(BLINKER_F("print: "), n);
    
    if (strlen(n) <= BLINKER_MAX_SEND_SIZE || \
        !needCheckLength)
    {
        BLINKER_LOG_FreeHeap();
        conn.print(n, isCheck);
    }
    else {
        BLINKER_ERR_LOG(BLINKER_F("SEND DATA BYTES MAX THAN LIMIT!"));
    }
}

template <class Transp>
void BlinkerProtocol<Transp>::begin()
{
    BLINKER_LOG(BLINKER_F(""));
    #if defined(BLINKER_NO_LOGO)
        BLINKER_LOG(BLINKER_F("Blinker v"BLINKER_VERSION"\n"
                    "    Give Blinker a Github star, thanks!\n"
                    "    => https://github.com/blinker-iot/blinker-library\n"));
    #elif defined(BLINKER_LOGO_3D)
        BLINKER_LOG(BLINKER_F("\n"
            " ____    ___                __                       \n"
            "/\\  _`\\ /\\_ \\    __        /\\ \\               v"BLINKER_VERSION"\n"
            "\\ \\ \\L\\ \\//\\ \\  /\\_\\    ___\\ \\ \\/'\\      __   _ __   \n"
            " \\ \\  _ <'\\ \\ \\ \\/\\ \\ /' _ `\\ \\ , <    /'__`\\/\\`'__\\ \n"
            "  \\ \\ \\L\\ \\\\_\\ \\_\\ \\ \\/\\ \\/\\ \\ \\ \\\\`\\ /\\  __/\\ \\ \\/  \n"
            "   \\ \\____//\\____\\\\ \\_\\ \\_\\ \\_\\ \\_\\ \\_\\ \\____\\\\ \\_\\  \n"
            "    \\/___/ \\/____/ \\/_/\\/_/\\/_/\\/_/\\/_/\\/____/ \\/_/  \n"
            "   Give Blinker a Github star, thanks!\n"
            "   => https://github.com/blinker-iot/blinker-library\n"));
    #else
        BLINKER_LOG(BLINKER_F("\n"
            "   ___  ___      __    v"BLINKER_VERSION"\n"
            "  / _ )/ (_)__  / /_____ ____\n"
            " / _  / / / _ \\/  '_/ -_) __/\n"
            "/____/_/_/_//_/_/\\_\\\\__/_/   \n"
            "Give Blinker a github star, thanks!\n"
            "=> https://github.com/blinker-iot/blinker-library\n"));
    #endif
}

template <class Transp>
void BlinkerProtocol<Transp>::run()
{
    if (!_isInit)
    {
        if (conn.init() && BApi::ntpInit())
        {
            _isInit =true;
            _disconnectTime = millis();

            // BApi::loadOTA();
            
            BLINKER_LOG_ALL(BLINKER_F("MQTT conn init success"));
        }
    }
    else {
        if (((millis() - _disconnectTime) > 60000 && \
            _disconnectCount) || _disconnectCount >= 12)
        {
            BLINKER_LOG_ALL(BLINKER_F("device reRegister"));
            BLINKER_LOG_FreeHeap();
            
            if (BLINKER_FreeHeap() < 15000) {
                conn.disconnect();
                return;
            }

            BLINKER_LOG_FreeHeap();

            if (conn.reRegister()) {
                _disconnectCount = 0;
                _disconnectTime = millis();
            }
            else {
                _disconnectCount = 0;
                _disconnectTime = millis() - 10000;
            }
        }

        BApi::ntpInit();
    }

    if ((millis() - _refreshTime) >= BLINKER_ONE_DAY_TIME * 2 * 1000)
    {
        conn.disconnect();

        BLINKER_LOG_ALL(BLINKER_F("device reRegister"));
        BLINKER_LOG_FreeHeap();

        if (BLINKER_FreeHeap() < 15000) {
            conn.disconnect();
            return;
        }

        BLINKER_LOG_FreeHeap();

        if (conn.reRegister()) {
            _refreshTime = millis();
        }
    }

    if (WiFi.status() != WL_CONNECTED)
    {        
        if ((millis() - _reconTime) >= 10000 || \
            _reconTime == 0 )
        {
            _reconTime = millis();
            BLINKER_LOG(BLINKER_F("WiFi disconnected! reconnecting!"));
            WiFi.reconnect();
        }

        return;
    }

    bool conState = conn.connected();

    switch (state)
    {
        case CONNECTING :
            if (conn.connect()) {
                state = CONNECTED;
                _disconnectCount = 0;
            }
            else {
                if (_isInit) {
                    if (_disconnectCount == 0) {
                        _disconnectCount++;
                        _disconnectTime = millis();
                        _disFreshTime = millis();
                    }
                    else {
                        // if ((millis() > _disFreshTime) && (millis() - _disFreshTime) >= 5000) {
                        if ((millis() - _disFreshTime) >= 5000) {
                            _disFreshTime = millis();
                            _disconnectCount++;

                            if (_disconnectCount > 12) _disconnectCount = 12;

                            BLINKER_LOG_ALL(BLINKER_F("_disFreshTime: "), _disFreshTime);
                            BLINKER_LOG_ALL(BLINKER_F("_disconnectCount: "), _disconnectCount);
                        }
                    }
                }
            }
            break;
        case CONNECTED :
            if (conState)
            {
                checkAvail();
                if (isAvail)
                {
                    BApi::parse(dataParse());
                }
                if (isAvail)
                {
                    // BApi::aliParse(conn.lastRead());
                    String vaName = BLINKER_F("vAssistant");
                    if (STRING_contains_string(conn.lastRead(), vaName))
                    {
                        flush();
                    }
                }
                if (checkAliAvail())
                {
                    // BApi::aliParse(conn.lastRead());
                }
            }
            else
            {
                conn.disconnect();
                state = CONNECTING;
                if (_isInit)
                {
                    if (_disconnectCount == 0)
                    {
                        _disconnectCount++;
                        _disconnectTime = millis();
                        _disFreshTime = millis();
                    }
                    else
                    {
                        if ((millis() - _disFreshTime) >= 5000)
                        {
                            _disFreshTime = millis();
                            _disconnectCount++;

                            if (_disconnectCount > 12) _disconnectCount = 12;
                            
                            BLINKER_LOG_ALL(BLINKER_F("_disFreshTime: "), _disFreshTime);
                            BLINKER_LOG_ALL(BLINKER_F("_disconnectCount: "), _disconnectCount);
                        }
                    }
                }
            }
            break;
        case DISCONNECTED :
            conn.disconnect();
            state = CONNECTING;
            break;
    }

    checkAutoFormat();
}

#endif