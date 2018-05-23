BEGIN {
    Output = 0
}
$2 == "Command" { Output = 1 }
END { print Output }

