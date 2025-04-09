#!/usr/bin/env nu

def main [test: int] {
	let out = $"./out/$($test).txt"
	let expected = $"./expected/($test).txt"


	let output = (./build/driver_c $test) | complete

	echo $output.stdout | save -f $out

	if $output.exit_code != 0 {
		bat $out
		error make {msg: "Failed to run", }
		return 1
	}

	bat $out


	if ($expected | path exists) {
		delta $out $expected
	} else {
		error make {msg: $"No expected file found ($expected)", }
	}
}

