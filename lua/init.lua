local wifiConected = false

-- This will not work because HTTPS is REQUIRED, consider using the proxy-pass or build with TLS support
local OPENAI_URL = "http://api.openai.com/v1/chat/completions"
local OPENAI_API_KEY = "YOUR_API_KEY_HERE"

local MY_WIFI = {
    ssid = "YOUR_SSID_HERE",
    pwd = "YOUR_PASSWORD_HERE"
}

local function setup()
    print("Starting up...")

    wifi.setmode(wifi.STATION)
    wifi.sta.config(MY_WIFI)
    wifi.eventmon.register(wifi.eventmon.STA_CONNECTED, function (event)
        print("Connected to: "..event.SSID)
    end)

    wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, function (event)
        wifiConected = true
        print("Got IP: "..event.IP)
    end)
end

local function parseStory(body)
    local response = sjson.decode(body)
    return response.choices[1].message.content
end

function GetStoryFromGPT(storyHandler)
    local prompt = "生成一篇童话，200汉字以内"
    local headers = "Authorization: Bearer "..OPENAI_API_KEY.."\r\nContent-Type: application/json\r\n"
    local body = "{\"model\": \"gpt-3.5-turbo\",\"messages\": [{\"role\": \"user\",\"content\": \""..prompt.."\"}]}"

    http.post(OPENAI_URL, headers, body, function(code, data)
        if (code < 0) then
            print("HTTP request failed")
        else
            storyHandler(parseStory(data))
        end
    end)
end

function Main()
    if not wifiConected then
        return
    end
    
    GetStoryFromGPT(function (story)
        print(story)
    end)
end

setup()
