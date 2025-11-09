# PowerShell script to modify LmInternalDatabase.xml
# Assign VlmcsdIndex="0" to the CsvlkItem with DisplayName="Windows Server 2025"
# And renumber the other CsvlkItem with VlmcsdIndex from 1 to n

param (
    [string]$FilePath = "LicenseManager/LmInternalDatabase.xml"
)

# Load the XML file
[xml]$xml = Get-Content -Path $FilePath -Encoding UTF8

# Select the CsvlkItems
$csvlkItems = $xml.KmsData.CsvlkItems.CsvlkItem

# Find the CsvlkItem for Windows Server 2025
$win2025 = $csvlkItems | Where-Object { $_.DisplayName -eq "Windows Server 2025" }

if ($win2025) {
    # Assign VlmcsdIndex="0"
    $win2025.SetAttribute("VlmcsdIndex", "0")
    Write-Host "Assigned VlmcsdIndex='0' to $($win2025.DisplayName)"
}
else {
    Write-Warning "CsvlkItem with DisplayName='Windows Server 2025' not found."
    exit
}

# Collect the other CsvlkItem with VlmcsdIndex defined
$othersWithIndex = $csvlkItems | Where-Object { $_.VlmcsdIndex -and $_ -ne $win2025 }

# Sort by current VlmcsdIndex (as integer)
$sortedOthers = $othersWithIndex | Sort-Object { [int]$_.VlmcsdIndex }

# Renumber from 1 to n
$index = 1
foreach ($item in $sortedOthers) {
    $oldIndex = $item.VlmcsdIndex
    $item.SetAttribute("VlmcsdIndex", $index.ToString())
    Write-Host "Renumbered $($item.DisplayName) from $oldIndex to $index"
    $index++
}

# Save the file
$xml.Save($FilePath)
Write-Host "Changes saved to $FilePath"