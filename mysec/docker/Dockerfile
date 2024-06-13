# Use an official Ubuntu as a parent image
FROM  vcatechnology/linux-mint

# Update the package repository and install make and gcc
RUN apt-get update && apt-get install -y \
    make \
    gcc \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /workspace

# Copy the current directory contents into the container
COPY . /workspace

# Define the command to run when the container starts
CMD ["/bin/bash"]

