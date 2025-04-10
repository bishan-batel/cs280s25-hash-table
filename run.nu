#!/usr/bin/env nu

def main [test: int] {
	if $test == 100 {
		return (all_tests)
	}

	let out = $"./out/($test).txt"
	let expected = $"./expected/($test).txt"


	let output = (./build/driver_c $test) | complete

	echo $output.stdout | save -f $out

	if $output.exit_code != 0 {
		bat $out
		error make {msg: "Failed to run", }
		return 1
	}

	#	cat $out

	print "======="

	if ($expected | path exists) {
		diff $out $expected | save -f $"./out/($test).diff"
		cat $"./out/($test).diff"
	} else {
		error make {msg: $"No expected file found ($expected)", }
	}
}

def all_tests [] { 
	for i in 1..13 { 
		main $i
	}
}
