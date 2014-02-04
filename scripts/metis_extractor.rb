# Turnes ndmetis -dbglvl=64 output into a machine-readable csv
#
# First argument: a file containing the output of a ndmetis -dbglvl=64 call
#
# Creates a file named '${input}.csv' containing the same graph which was given as input only that each nodes id equals its rank of the given node order

unless File.exists? ARGV[0]
  puts 'File not found! Exiting!'
  exit
end

File.open("#{ARGV[0]}.csv", 'w+') do |export|
    export.puts "size,sizel,sizer,size_sep"
    File.open(ARGV[0]).each do |line|
        next unless line.start_with? "Nvtxs: "
        tokens = line.gsub(/[,\[\]]/, " ").split " "
        export.puts tokens[1..-1].join ","
    end
end