#!/usr/bin/env ruby

IGNORED_UNDEFINED_SYMBOLS = %w[
  qh_deregister_handler
]

Dir.chdir(File.expand_path("../..", __FILE__))

symbols = `nm -gu #{ENV["SONAME"]} | grep -v GLIBC`.lines.map { |line|
  line.strip.split(" ", 2)
}.select { |type, symbol|
  type == "U" # undefined symbols only
}.map { |type, symbol|
  symbol
}

symbols.each do |symbol|
  `grep -qr #{symbol} /usr/include/nagios`
  next if $?.success?
  next if IGNORED_UNDEFINED_SYMBOLS.include?(symbol)
  puts "Undefined symbol reference `#{symbol}' does not appear to belong to nagios."
end
