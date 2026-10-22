[CmdletBinding()]
param(
    [string]$Destino = (Join-Path ([Environment]::GetFolderPath('MyDocuments')) 'Instro-Natural'),
    [string]$Bancos = 'todos',
    [string]$ManifestPath,
    [switch]$Listar
)

$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
$releaseUrl = 'https://github.com/tuofxit/Pluguin.Inst/releases/download/v0.4.0/'
$selectedBanks = @($Bancos.Split(',') | ForEach-Object { $_.Trim().ToLowerInvariant() })
foreach ($bank in $selectedBanks) {
    if ($bank -notin @('todos','nenhum','piano','violoes','guitarras','baixo','orquestra','extras','urban')) {
        throw ('Banco desconhecido: ' + $bank)
    }
}

if ($ManifestPath) {
    $manifest = Get-Content -LiteralPath $ManifestPath -Raw | ConvertFrom-Json
} else {
    $manifest = Invoke-RestMethod -Uri ($releaseUrl + 'manifest.json')
}
if ($Listar) {
    $manifest.groups.PSObject.Properties | ForEach-Object { Write-Output ($_.Name + ': ' + $_.Value) }
    return
}

$installRoot = [IO.Path]::GetFullPath($Destino).TrimEnd('\', '/')
if ($installRoot -eq [IO.Path]::GetPathRoot($installRoot).TrimEnd('\', '/')) {
    throw 'Escolha uma pasta para o Instro, e nao a raiz de uma unidade.'
}
$cache = Join-Path $installRoot 'Downloads'
New-Item -ItemType Directory -Path $installRoot, $cache -Force | Out-Null

$chosen = @($manifest.assets | Where-Object {
    $_.group -eq 'base' -or (
        $_.group -notin @('codigo','demo') -and (
            'todos' -in $selectedBanks -or $_.group -in $selectedBanks
        )
    )
})

foreach ($asset in $chosen) {
    if (-not $asset.url.StartsWith('https://github.com/tuofxit/Pluguin.Inst/releases/download/', [StringComparison]::Ordinal)) {
        throw ('URL fora do repositorio: ' + $asset.file)
    }
    if ([IO.Path]::GetFileName($asset.file) -ne $asset.file) {
        throw 'Nome de arquivo invalido no manifesto.'
    }
    $target = Join-Path $cache $asset.file
    $valid = (Test-Path -LiteralPath $target) -and (
        (Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash -eq $asset.sha256
    )
    if (-not $valid) {
        Write-Host ('Baixando ' + $asset.file + ' (' + [Math]::Round($asset.bytes / 1MB) + ' MB)...')
        $part = $target + '.part'
        $complete = $false
        for ($attempt = 1; $attempt -le 3; $attempt++) {
            try {
                Invoke-WebRequest -UseBasicParsing -Uri $asset.url -OutFile $part
                if ((Get-FileHash -LiteralPath $part -Algorithm SHA256).Hash -ne $asset.sha256) {
                    throw ('Checksum incorreto: ' + $asset.file)
                }
                Move-Item -LiteralPath $part -Destination $target -Force
                $complete = $true
                break
            } catch {
                if ($attempt -eq 3) { throw }
                Write-Host 'Falha no download. Tentando novamente...'
                Start-Sleep -Seconds (2 * $attempt)
            }
        }
        if (-not $complete) { throw ('Nao foi possivel baixar ' + $asset.file) }
    } else {
        Write-Host ('Download ja conferido: ' + $asset.file)
    }
    $extractTo = [IO.Path]::GetFullPath((Join-Path $installRoot $asset.destination))
    if ($extractTo -ne $installRoot -and -not $extractTo.StartsWith($installRoot + '\', [StringComparison]::OrdinalIgnoreCase)) {
        throw 'Destino fora da pasta de instalacao.'
    }
    New-Item -ItemType Directory -Path $extractTo -Force | Out-Null
    Write-Host ('Extraindo ' + $asset.file + '...')
    Expand-Archive -LiteralPath $target -DestinationPath $extractTo -Force
}

Write-Host ''
Write-Host ('Pronto! Arquivos em: ' + $installRoot)
Write-Host ('Aplicativo: ' + (Join-Path $installRoot 'Standalone\Instro Natural.exe'))
Write-Host ('Plugin VST3: ' + (Join-Path $installRoot 'VST3\Instro Natural.vst3'))
Write-Host ('Biblioteca: ' + (Join-Path $installRoot 'Banks'))
Write-Host 'No Ableton, use a pasta VST3 como pasta personalizada, ou copie o pacote .vst3 para a pasta VST3 do sistema.'
Write-Host 'Na interface do plugin, selecione a pasta Banks e aguarde Pronto antes de tocar.'
Write-Host 'Os ZIPs conferidos ficam em Downloads. Voce pode apagar essa pasta depois de conferir a instalacao.'
