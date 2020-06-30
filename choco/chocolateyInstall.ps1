$packageName = 'demohelper'
$url64 = 'https://github.com/stefankueng/demohelper/releases/download/$MajorVersion$.$MinorVersion$.$MicroVersion$/DemoHelper-$MajorVersion$.$MinorVersion$.$MicroVersion$.zip'
$checksum64 = '$checksum64$'
$checksumType64 = 'sha256'
$toolsDir = "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"
try {
  Install-ChocolateyZipPackage -PackageName '$packageName' `
                               -UnzipLocation "$toolsDir" `
                               -Url64bit "$url64" `
                               -Checksum64 "$checksum64" `
                               -ChecksumType64 "$checksumType64"
} catch {
  throw $_.Exception
}