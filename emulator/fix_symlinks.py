import os

def fix_file(file):
	if os.path.islink(file):
		print(f'{file} already fixed, skipping')
		return

	with open(file) as f: filename = f.read()
	os.remove(file)
	os.symlink(os.getcwd() + '/../' + filename, file)
	print(f'Fixed {file}')

if __name__ == '__main__':
	if os.listdir('../imgui'):
		for file in os.listdir('src/Gui/imgui'): fix_file(f'src/Gui/imgui/{file}')
		print('All done!')
	else:
		print('''\
Before running this script, please run these commands in the root of the repository directory:
git submodule init
git submodule update

If you did not clone the repository with Git, please delete the repository directory and run:
git clone https://github.com/fxesdev/CasioEmuX.git --recursive
then re-run this script.\
''')
