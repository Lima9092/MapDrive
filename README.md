# MapDrive (MapDriveNoDomain.exe)
Map a drive in a non-domain environment. The app will query the users UPN first and if it can't find that the standard user account. It prompts the user for their password and mapps a drive based on a config.txt file running in the same folder as the app
# Config.txt
drive_letter=Z:
network_path=\\server\share
